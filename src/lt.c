#include <sys/eventfd.h>
#include "config.h"
#include "lt.h"
#include "setnonblocking.h"
#include "connect.h"
#include "worker.h"

int create_eventfd()
{
	  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	  if (evtfd < 0)
	  {
		printf("Failed in eventfd\n");
		return -1;
	  }
	  return evtfd;
}
 void wakeup()
 {
	 int one = 1;
	 ssize_t n = send(wakeupfd, &one, sizeof(one), 0);
	 if(n != sizeof(one)){
		 printf("wakeup writes %d bytes instead of 4\n", n);
	 }
 }

 void handle_wake_read()
 {
	  int one = 1;
	  ssize_t n = recv(wakeupfd, &one, sizeof (one), 0);
	  if (n != sizeof(one))
	  {
		  printf("handle_wake_read reads %d bytes instead of 4\n");
	  }
 }

int epoll_add_event(int ep, int fd, void* conn){
	addref((conn_rec *)conn);
	struct epoll_event ee;
	ee.events = EPOLLIN;
	ee.data.ptr = conn;
	if(epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ee) == -1){
		return -1;
	}
	return 0;
}

int epoll_mod_event(int ep, int fd, void* conn, uint32_t event){
	struct epoll_event ee;
	ee.events = event;
	ee.data.ptr = conn;
	if(epoll_ctl(ep, EPOLL_CTL_MOD, fd, &ee) == -1){
		return -1;
	}
	return 0;
}

int epoll_del_event(int ep, int fd, void* conn, uint32_t event){
	struct epoll_event ee;
	ee.events = event;
	ee.data.ptr = conn;
	if(epoll_ctl(ep, EPOLL_CTL_DEL, fd, &ee) == -1){
		return -1;
	}
	return 0;
}

void* epoll_loop(void *param)
{
	int i;
	unsigned nfds;
	int sockfd;
	int ret;
	char completionPacket[DATA_BUFSIZE];
	struct epoll_event events[MAX_EPOLL_EVENT_COUNT];

	printf("io线程正在运行 tid=%lu!\n", pthread_self());

	while(1)
	{
		nfds = epoll_wait(epfd, events, MAX_EPOLL_EVENT_COUNT, -1);
		for(i = 0; i < nfds; i++)
		{
			conn_rec *c = (conn_rec *)events[i].data.ptr;
			sockfd =  c->fd;
			if(sockfd == listenfd)
			{
				int connfd;
				struct sockaddr_in clientaddr = {0};
				socklen_t clilen = sizeof(clientaddr);

				connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
				if(connfd == -1){
					printf("套接字accept失败! 错误码:%d\n", errno);
					continue;
				}

				setnonblocking(connfd);
				int buf_size=32*1024;
				setsockopt(connfd, SOL_SOCKET,SO_RCVBUF,(const char*)&buf_size,sizeof(int));
				setsockopt(connfd, SOL_SOCKET,SO_SNDBUF,(const char*)&buf_size,sizeof(int));

				int timeout=10000;//1秒
				setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(int));
				setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(int));

				const char *remote_ip = inet_ntoa(clientaddr.sin_addr);
				int  remote_port = ntohs(clientaddr.sin_port);
				printf("accept connect ip=%s port=%d\n", remote_ip, remote_port);

				conn_rec *c = create_conn(connfd, remote_ip, remote_port);
				epoll_add_event(epfd, connfd, c);
				add_connect(c);
			}
			else if(sockfd == wakeupfd && events[i].events & EPOLLIN){
				handle_wake_read();
			}
			else if(events[i].events & EPOLLIN)
		    {
				//收到头部4个字节或完整的数据包
				printf("触发读事件\n");
				while(1) {
					ret = packet_recv(c);
					if( ret == RECV_COMPLATE){
						if(has_complate_packet(c)){
							printf("c->recv_queue %d\n",c->recv_queue->size);
							push_packet(c->recv_queue, c);
							c->recv_queue = buffer_queue_init(c->pool);
							pthread_cond_signal(&thread_cond);
						}
					}
					else {
						if( ret == RECV_FAILED){
							printf("接收失败 FAILED\n");
							close_connect(c);
						}
						break;
					}
				}
		    }
			else if(events[i].events & EPOLLOUT)
			{
				ret = packet_send(c);
				if(ret == SEND_COMPLATE){
					buffer_queue_detroy(c->send_queue);
					c->send_queue = NULL;
					epoll_mod_event(epfd, c->fd, c, EPOLLIN);
				}
				else if(ret == SEND_FAILED){
					close_connect(c);
				}
		    }
		    else if((events[i].events & EPOLLHUP) || (events[i].events & EPOLLERR))
		    {
		    	//服务端出错触发
		    	close_connect(c);
		    }
		    else if(events[i].events & EPOLLRDHUP)
		    {
		    	//客户端关闭触发EPOLLIN和EPOLLRDHUP
		    	close_connect(c);
		    }
			pthread_mutex_lock(&result_queue_mutex);
			struct job_node_t   *node = result_queue.p_head;
			while(node){
				conn_rec *c = node->con;
				if(c->send_queue){
					if(c->send_queue->size > 0){
						buffer_queue_write_ex(c->send_queue, node->buf_queue);
						continue;
					}
				}

				buffer_queue_detroy(c->send_queue);
				c->send_queue = node->buf_queue;
				ret = packet_send(c);
				if(ret == SEND_AGAIN){
					epoll_mod_event(epfd, c->fd, c, EPOLLOUT);
				}
				else if(ret == SEND_FAILED){
					close_connect(c);
				}

				node = remove_result(node);
			}
			pthread_mutex_unlock(&result_queue_mutex);
		}
	 }

	 return NULL;
}

int length_to_read(conn_rec *c)
{
	if (c->recv_queue->size < sizeof(int))
		return sizeof(int)-c->recv_queue->size;
	else{
		int *len = (int *)c->recv_queue->p_head->buffer;
		if(*len >= 100000){
			//data length is too large
			printf("data length is too large\n");
			c->aborted = 1;
			return -1;
		}else if(*len <= 0){
			printf("data length is error\n");
			c->aborted = 1;
			return -1;
		}
		return *len - c->recv_queue->size;
	}
}

int has_complate_packet(conn_rec *c)
{
	if (c->recv_queue->size < sizeof(int))
		return 0;
	int *len = (int *)(c->recv_queue->p_head->buffer);
	return c->recv_queue->size >= *len;
}

int packet_recv(conn_rec *c)
{
	int aborted = 0;
	int recvlen = 0;
	int total_recvlen = 0;
	int needlen = length_to_read(c);
	if(needlen == -1){
		return RECV_FAILED;
	}
	while(1) {
		struct buffer_packet_t *packet = buffer_queue_last(c->recv_queue);
		int begin = DATA_BUFSIZE - packet->remain_size;
		int readlen = needlen-total_recvlen<packet->remain_size?needlen-total_recvlen:packet->remain_size;
		aborted = sock_recv(c, packet->buffer+begin, readlen, &recvlen);
		packet->remain_size -= recvlen;
		c->recv_queue->size += recvlen;
		total_recvlen += recvlen;
		if(aborted == 1){
			return RECV_FAILED;
		}
		//判断本次接收是否完成
		if(recvlen < readlen){
			return RECV_AGAIN;
		}
		if(total_recvlen >=needlen){
			return RECV_COMPLATE;
		}
	}
	return total_recvlen>=needlen?RECV_COMPLATE:RECV_AGAIN;
}

int sock_recv(conn_rec *c, char *recvbuf, int recvlen, int *realrecvlen)
{
	int fd = c->fd;
	int ret = 0;
	int len = 0;
	while(recvlen>0){
		ret = recv(fd, recvbuf + len, recvlen, 0);
		if(ret > 0){
			len += ret;
			recvlen -= len;
		}else{
			break;
		}
	}
	*realrecvlen = len;

	if(ret == 0){
		c->aborted = 1;
	}
	else if(ret < 0){
		if(errno != EINTR && (errno != EWOULDBLOCK && errno != EAGAIN)){
			/*network error*/
			c->aborted = 1;
		}
	}
	return c->aborted;
}

int packet_send(conn_rec *c)
{
	int aborted = 0;
	char *buf = NULL;
	int len = 0;
	int sendlen = 0;
	struct buffer_packet_t *packet = NULL;
	while (1) {
		packet = buffer_queue_head(c->send_queue);
		if(packet == NULL){
			break;
		}
		buf = packet->buffer + packet->send_size;
		len = DATA_BUFSIZE-(packet->send_size+ packet->remain_size);
		aborted = sock_send(c, buf, len, &sendlen);
		packet->send_size +=sendlen;
		c->send_queue->size -=sendlen;
		if(aborted == 1){
			return SEND_FAILED;
		}
		if(sendlen< len){
			return SEND_AGAIN;
		}
		buffer_queue_pop(c->send_queue);
	}
	return SEND_COMPLATE;
}

int sock_send(conn_rec *c, char *sendbuf, int sendlen, int* realsendlen)
{
	char *buf = NULL;
	int fd = c->fd;
	int ret = 0;
	int len = 0;
	int remain_len = sendlen;

	while(remain_len>0){
		buf = sendbuf + len;
		ret = send(c->fd, buf, remain_len, 0);
		if(ret > 0){
			len += ret;
			remain_len =  sendlen - len;
		}else{
			break;
		}
	}
	*realsendlen = len;

	if(ret == 0){
		c->aborted = 1;
	}
	else if(ret < 0){
		if(errno != EINTR && (errno != EWOULDBLOCK && errno != EAGAIN)){
			c->aborted = 1;
		}
	}
	return c->aborted;
}

void close_connect(conn_rec *c)
{
	printf("关闭套接字\n");
	epoll_del_event(epfd, c->fd, c, EPOLLIN);
	close(c->fd);
	if(!deref(c)){
		pthread_mutex_lock(&conn_list_lock);
		printf("移除连接\n");
		remove_connect(c);
		pthread_mutex_unlock(&conn_list_lock);
	}
}
