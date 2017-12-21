#include "setnonblocking.h"
#include "config.h"
#include "lt.h"
#include "connect.h"
#include "heart.h"
#include "worker.h"

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

int main(){
	 int ret = 0, i = 0;
	 int option = 0;
	 struct epoll_event ev;

	 if(initlog()){
		 return 0;
	 }
	 apr_pool_initialize();
	 atomic_init(&server_stop);

	 if(apr_pool_create(&server_rec, NULL) != APR_SUCCESS){
		 zlog_error(z_cate, "内存池初始化失败!");
		 return 0;
	 }

	 signal(SIGHUP, sigroutine);
	 signal(SIGINT, sigroutine);
	 signal(SIGQUIT, sigroutine);

	 if(pthread_mutex_init(&conn_list_mutex, NULL) != 0){
		 zlog_error(z_cate, "互斥锁创建失败!");
		 return 0;
	 }
	 epfd = epoll_create(MAX_EPOLL_EVENT_COUNT);

	 init_global_queue();

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

	 //开始监听
	 if (listen(listenfd, LISTENQ) == -1){
		 zlog_error(z_cate, "监听失败! 错误码: %d", errno);
		 return -1;
	 }

	 conn_rec *tmp = create_conn(listenfd, NULL, 0);
	 epoll_add_event(epfd, listenfd, tmp);

	 wakeupfd = create_eventfd();
	 tmp = create_conn(wakeupfd, NULL, 0);
	 epoll_add_event(epfd, wakeupfd, tmp);

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
	for(;;){
		 sleep(100);
	 }
	 close(listenfd);
}

