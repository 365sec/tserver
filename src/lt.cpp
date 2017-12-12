#include <sys/socket.h>
 #include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include "lt.h"
#include "config.h"
#include "setnonblocking.h"
#include "connect.h"

int epoll_add_event(int ep, int fd, void* conn){
	epoll_event ee;
	ee.events = EPOLLIN;
	ee.data.ptr = conn;
	if(epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ee) == -1){
		return -1;
	}
	return 0;
}

void* epoll_loop(void *param)
{
	int i;
	unsigned nfds;
	int sockfd;

	struct epoll_event events[MAX_EPOLL_EVENT_COUNT];

	printf("io线程正在运行 tid=%lu!\n", pthread_self());

	while(true)
	{
		//等待epoll事件的发生
		nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENT_COUNT, -1);
		printf("nfds=%d\n", nfds);
		for(i = 0; i < nfds; i++)
		{
			conn_rec *c = (conn_rec *)events[i].data.ptr;
			sockfd =  c->fd;
			if(sockfd == listenfd)
			{
				int connfd;
				struct sockaddr_in clientaddr = {0};
				socklen_t clilen = sizeof(clientaddr);

				connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
				if(connfd == -1){
					printf("套接字accept失败! 错误码:%d\n", errno);
					continue;
				}

				setnonblocking(connfd);

				//设置接收缓冲区
				int recvbuf=32*1024;//设置为32K
				setsockopt(connfd, SOL_SOCKET,SO_RCVBUF,(const char*)&recvbuf,sizeof(int));
				//设置发送缓冲区
				int sendbuf=32*1024;//设置为32K
				setsockopt(connfd, SOL_SOCKET,SO_SNDBUF,(const char*)&sendbuf,sizeof(int));

				int timeout=10000;//1秒
				//发送时限
				setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(int));
				//接收时限
				setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(int));

				const char *remote_ip = inet_ntoa(clientaddr.sin_addr);
				int  remote_port = ntohs(clientaddr.sin_port);
				printf("accept connect ip=%s port=%d\n", remote_ip, remote_port);

				conn_rec *c = create_conn(connfd, remote_ip, remote_port);
				epoll_add_event(epfd, connfd, c);
			}
			else if(events[i].events & EPOLLIN)
		    {
				char data[101] = {0};
				int ret = recv(sockfd, data, 100, 0);

		    }
			else if(events[i].events & EPOLLOUT)  //write event
			{

		    }
		    else if(events[i].events & EPOLLHUP) //huging up
		    {

		    }
		    else
		    {
			   printf("[warming]other epoll event: %d", events[i].events);
		    }
		}
	 }

	 return NULL;
}

