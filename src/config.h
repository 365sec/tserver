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
struct conn_rec_list_t;

#define SERV_PORT  5678 // 服务器端口
#define LISTENQ          128   // listen sock 参数
#define MAX_EPOLL_EVENT_COUNT      1024   // 同时监听的 epoll 数
#define EPOLL_LOOP_THREAD_COUNT  1   //IO线程数目
#define WORK_THREAD_COUNT   2 //工作线程数目
#define DATA_BUFSIZE 4096
#define ROOT_SERVER_IP "127.0.0.1"
#define ROOT_SERVER_PORT 6789
#define MSGHEADER_LENGTH sizeof(int)   //头部的长度所占字节数
#define COMMANDTHREAD_TIMEOUT 5			//指令监控线程的轮训间隔时间
#define SEND_FAILED 0
#define SEND_AGAIN 1
#define SEND_COMPLATE 2

#define RECV_FAILED 0
#define RECV_AGAIN 1
#define RECV_COMPLATE 2

extern zlog_category_t *z_cate;
extern int server_stop;
extern int epfd;
extern int listenfd;
extern int c_fd;
extern apr_pool_t *server_rec;
extern struct conn_rec_list_t conn_list;
extern pthread_mutex_t conn_list_mutex;
extern int wakeupfd;
extern char *db_host;
extern char *db_user;
extern char *db_password;
extern char *db_dbname;
extern char *db_schema;
extern char *db_socket;
extern int db_port;

#endif
