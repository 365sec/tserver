#include "setnonblocking.h"
#include "config.h"
#include "lt.h"
#include "connect.h"
#include "heart.h"
#include "worker.h"
#include "apr_strings.h"

void accept_command();
void sigroutine(int dunno)
{
	switch(dunno){
	case 1:
		printf("Get a signal -- SIGHUP ");
		break;
	case 2:
		printf("Get a signal -- SIGINT ");
		break;
	case 3:
		printf("Get a signal -- SIGQUIT ");
		break;
	}
}

int initlog()
{
	char buf[256] = {0};
	char path[256] = {0};
	sprintf(path, "%s/zlog.conf", getcwd(buf, sizeof(buf)));
	int rc = zlog_init(path);
	printf("%s\n", path);
	if (rc) {
		printf("日志模块初始化失败.\n");
		return -1;
	}
	z_cate = zlog_get_category("tserver");
	if (!z_cate) {
		printf("获取日志分类失败.\n");
		zlog_fini();
		return -1;
	}
	return 0;
}
 void register_signal()
 {
	 signal(SIGHUP, sigroutine);
	 signal(SIGINT, sigroutine);
	 signal(SIGQUIT, sigroutine);
 }

int main(){
	 int ret = 0, i = 0;
	 int option = 0;
	 struct epoll_event ev;

	 if(initlog()){
		 return 0;
	 }
	 register_signal();
	 atomic_init(&server_stop);

	 apr_pool_initialize();
	 if(apr_pool_create(&server_rec, NULL) != APR_SUCCESS){
		 zlog_error(z_cate, "内存池初始化失败!");
		 return 0;
	 }

	 if(pthread_mutex_init(&conn_list_mutex, NULL) != 0){
		 zlog_error(z_cate, "互斥锁创建失败!");
		 return 0;
	 }
	 epfd = epoll_create(MAX_EPOLL_EVENT_COUNT);

	 init_global_queue();
	 parser_bson_init();

	 struct sockaddr_in serveraddr;
	 if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		 zlog_error(z_cate, "监听套接字创建失败!");
		 return 0;
	 }
	 option = 1;
	 setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  &option, sizeof(option));
	 memset(&serveraddr, 0, sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_port = htons(SERV_PORT);
	 serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	 if (bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr))  == -1){
		 zlog_error(z_cate, "端口绑定失败! 错误码: %d", errno);
		 return -1;
	 }



	 conn_rec *tmp = create_conn(listenfd, NULL, 0);
	 epoll_add_event(epfd, listenfd, tmp);

	 wakeupfd = create_eventfd();
	 tmp = create_conn(wakeupfd, NULL, 0);
	 epoll_add_event(epfd, wakeupfd, tmp);

	 //开始监听
	 if (listen(listenfd, LISTENQ) == -1){
		 zlog_error(z_cate, "监听失败! 错误码: %d", errno);
		 return -1;
	 }

	 pthread_t tid;
	 ret = pthread_create(&tid, NULL, heart_check, NULL);
	 if(ret != 0){
		 zlog_error(z_cate, "创建心跳线程失败! 错误码: %d", ret);
		   return -1;
	 }
	 ret = pthread_create(&tid, NULL, epoll_loop, NULL);
	 if(ret != 0){
		 zlog_error(z_cate, "创建io线程失败! 错误码: %d", ret);
		   return -1;
	 }
	 for(i = 0; i< WORK_THREAD_COUNT; i++){
		 ret = pthread_create(&tid, NULL, work_thread, NULL);
		 if(ret != 0){
			 zlog_error(z_cate, "创建work线程失败! 错误码: %d", ret);
			   return -1;
		 }
	 }
	accept_command();
	close(listenfd);
	zlog_fini();
}

void accept_command()
{
	int ret = 0, i = 0;
	struct sockaddr_in c_addr;

	c_addr.sin_family = AF_INET;
	c_addr.sin_addr.s_addr = inet_addr(ROOT_SERVER_IP);
	c_addr.sin_port = htons(ROOT_SERVER_PORT);
	int c_poll = epoll_create(MAX_EPOLL_EVENT_COUNT);
	for(;;){
		if(c_fd <= 0){
			c_fd = socket(AF_INET, SOCK_STREAM,0);
			if (-1 == c_fd)
			{
				zlog_error(z_cate, "创建客户端套接字失败!");
				return 0;
			}
		}
		ret = connect(c_fd, (struct sockaddr*)&c_addr,sizeof(c_addr));
		if (ret < 0){
			zlog_error(z_cate, "连接到上游服务器失败.");
			sleep(5);
			continue;
		}
		setnonblocking(c_fd);
		 conn_rec *s_con = create_conn(c_fd, ROOT_SERVER_IP, ROOT_SERVER_PORT);
		 s_con->read_callback = NULL;
		 s_con->close_callback = NULL;
		 epoll_add_event(c_poll, c_fd, s_con);
		 struct epoll_event events[100];
		 for(;;){
			 int n = epoll_wait(c_poll, events, 100, 300);
			 for (i = 0; i < n; i++) {
				 conn_rec *c = (conn_rec *)events[i].data.ptr;
				 if(events[i].events & EPOLLIN){
					 handle_read(c);
				 }
				 else if(events[i].events & EPOLLOUT){
					 handle_write(c);
				 }
			 }
			 if(s_con && atomic_read(&s_con->aborted) != 0){
				 epoll_del_event(c_poll, s_con->fd, s_con, EPOLLIN);
				 close(s_con->fd);
				 release_connect(s_con);
				 s_con = NULL;
				 c_fd = 0;
				 break;
			 }else{
				 if(s_con->send_queue->size > 0){
					int ret = packet_send(s_con);
					if(ret == SEND_AGAIN){
						epoll_mod_event(c_poll, s_con->fd, s_con, EPOLLOUT);
					}
					else if(ret == SEND_FAILED){
						close_connect(s_con);
						break;
					}
					else if(ret == SEND_COMPLATE){
						buffer_queue_detroy(s_con->recv_queue);
						s_con->recv_queue = buffer_queue_init(s_con->pool);
					}
				 }
			 }
		 }
	}
}
