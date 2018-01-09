#include "setnonblocking.h"
#include "config.h"
#include "lt.h"
#include "connect.h"
#include "heart.h"
#include "worker.h"
#include "apr_strings.h"
#include "db_process.h"

void accept_command();

int initlog()
{
	char buf[256] = {0};
	char path[256] = {0};
	sprintf(path, "%s/zlog.conf", getcwd(buf, sizeof(buf)));
	int rc = zlog_init(path);
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

int init_server()
{
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

	 conn_list.conn_head = NULL;
	 conn_list.conn_last = NULL;
	 conn_list.size = 0;

	 init_work_thread();
	 parser_bson_init();

	 DBinit();
	 return 1;
}

int main(){

	int ret = 0, i = 0;
	int option = 0;
	struct epoll_event ev;

	if(initlog()){
		return 0;
	}

	if(!init_server())
		return 0;

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

	ret = pthread_create(&tid, NULL, db_read_thread, NULL);
	if(ret != 0){
	    zlog_error(z_cate, "创建指令读取线程失败! 错误码: %d", ret);
	    return -1;
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
				return;
			}
		}
		ret = connect(c_fd, (struct sockaddr*)&c_addr,sizeof(c_addr));
		if (ret < 0){
			//zlog_error(z_cate, "连接到上游服务器失败.");
			sleep(5);
			continue;
		}
		zlog_info(z_cate, "连接到上游服务器成功.");
		setnonblocking(c_fd);
		conn_rec *s_con = create_conn(c_fd, ROOT_SERVER_IP, ROOT_SERVER_PORT);
		s_con->read_callback = NULL;
		s_con->close_callback = NULL;
		epoll_add_event(c_poll, c_fd, s_con);
		struct epoll_event events[1];
		for(;;){
			int n = epoll_wait(c_poll, events, 1, 300);
			for (i = 0; i < n; i++) {
				conn_rec *c = (conn_rec *)events[i].data.ptr;
				if(events[i].events & EPOLLIN){
					zlog_debug(z_cate, "命令监听线程触发读事件.");
					handle_read(c);
				}
				else if(events[i].events & EPOLLOUT){
					zlog_debug(z_cate, "命令监听线程触发写事件.");
					handle_write(c);
				}
			}
			if(s_con && atomic_read(&s_con->aborted) != 0){
				zlog_debug(z_cate, "命令监听线程连接已断开.");
				break;
			}else{
				if(s_con->send_queue->size > 0){
					zlog_debug(z_cate, "命令监听线程开始发送数据.");
					int ret = packet_send(s_con);
					if(ret == SEND_AGAIN){
						epoll_mod_event(c_poll, s_con->fd, s_con, EPOLLOUT);
					}
					else if(ret == SEND_FAILED){
						break;
					}
					else if(ret == SEND_COMPLATE){
						buffer_queue_detroy(s_con->recv_queue);
						s_con->recv_queue = buffer_queue_init(s_con->pool);
					}
				 }
			}
		}
		epoll_del_event(c_poll, s_con->fd, s_con, EPOLLIN);
		close(s_con->fd);
		release_connect(s_con);
		s_con = NULL;
		c_fd = 0;
	}
}
