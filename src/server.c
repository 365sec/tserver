#include "setnonblocking.h"
#include "config.h"
#include "lt.h"
#include "connect.h"
#include "heart.h"

int main(){
	 int ret = 0, i = 0;
	 int option = 0;
	 struct epoll_event ev;  

	 apr_pool_initialize();

	 if(apr_pool_create(&server_rec, NULL) != APR_SUCCESS){
		 printf("内存池初始化失败!\n");
		 return 0;
	 }
	 if(pthread_mutex_init(&conn_list_lock, NULL) != 0){
		 printf("互斥锁创建失败!\n");
		 return 0;
	 }
	 epfd = epoll_create(MAX_EPOLL_EVENT_COUNT);

	 init_global_queue();

	 struct sockaddr_in serveraddr;
	 if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		 printf("监听套接字创建失败!\n");
		 return 0;
	 }
	 option = 1;
	 setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  &option, sizeof(option));
	 memset(&serveraddr, 0, sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_port = htons(SERV_PORT);
	 serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	 if (bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr))  == -1){
		 printf("端口绑定失败! 错误码: %d\n", errno);
		 return -1;
	 }

	 //开始监听
	 if (listen(listenfd, LISTENQ) == -1){
		 printf("监听失败! 错误码: %d\n", errno);
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
		   printf("创建心跳线程失败! 错误码: %d\n", ret);
		   return -1;
	 }
	 ret = pthread_create(&tid, NULL, epoll_loop, NULL);
	 if(ret != 0){
		   printf("创建io线程失败! 错误码: %d\n", ret);
		   return -1;
	 }
	 while(1){
		 sleep(100);
	 }
	 close(listenfd);
}

