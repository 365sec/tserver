#include <sys/socket.h>
 #include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "setnonblocking.h"
#include "config.h"
#include "lt.h"

int main(){
	 int ret = 0;
	 int option = 0;
	 struct epoll_event ev;  

	 epfd = epoll_create(MAX_EPOLL_EVENT_COUNT);

	 struct sockaddr_in serveraddr;
	 if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		 printf("监听套接字创建失败!\n");
		 return 0;
	 }

	 //setnonblocking(listenfd);
	 option = 1;
	 setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  &option, sizeof(option));
	 printf("listenfd=%d\n", listenfd);
	 memset(&serveraddr, 0, sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_port = htons(SERV_PORT);
	 serveraddr.sin_addr.s_addr = INADDR_ANY;

	 if (bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr))  == -1){
		 printf("端口绑定失败! 错误码: %d\n", errno);
		 return false;
	 }

	 //开始监听
	 if (listen(listenfd, LISTENQ) == -1){
		 printf("监听失败! 错误码: %d\n", errno);
		 return false;
	 }

	 conn_rec *c = create_conn(listenfd, NULL, 0);
	 epoll_add_event(epfd, listenfd, c);

	 pthread_t tids[EPOLL_LOOP_THREAD_COUNT];
	 for (int i = 0; i < EPOLL_LOOP_THREAD_COUNT; i++){
		 ret = pthread_create(&tids[i], NULL, epoll_loop, NULL);
		 if(ret != 0){
			   printf("创建io线程失败! 错误码: %d\n", ret);
			   return false;
		 }
	 }
	 while(true){
		 sleep(100);
	 }
	 close(listenfd);
}

