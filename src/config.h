#ifndef _CONFIG_H
#define _CONFIG_H

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
 #include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include "apr_pools.h"
#include "apr_thread_mutex.h"
#include "zlog.h"
#include "atomic.h"

struct conn_rec_t;

#define SERV_PORT  5678 // 服务器端口
#define LISTENQ          128   // listen sock 参数
#define MAX_EPOLL_EVENT_COUNT      1024   // 同时监听的 epoll 数
#define EPOLL_LOOP_THREAD_COUNT  1   //IO线程数目
#define WORK_THREAD_COUNT   2 //工作线程数目
#define DATA_BUFSIZE 4096

#define MSGHEADER_LENGTH sizeof(int)   //头部的长度所占字节数

#define SEND_FAILED 0
#define SEND_AGAIN 1
#define SEND_COMPLATE 2

#define RECV_FAILED 0
#define RECV_AGAIN 1
#define RECV_COMPLATE 2

extern zlog_category_t *z_cate;
extern atomic server_stop;
extern int epfd;
extern int listenfd;
extern apr_pool_t *server_rec;
extern struct conn_rec_t *conn_head;
extern struct conn_rec_t *conn_last;
extern pthread_mutex_t conn_list_mutex;
extern int wakeupfd;

#endif
