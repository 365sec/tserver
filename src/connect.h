#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "buffer_queue.h"
#include "config.h"
#include "atomic.h"

typedef struct conn_rec_t{
	apr_pool_t *pool;

	/* client ip address*/
	char *remote_ip;
	/*client port*/
	int remote_port;

	/*are we still talking*/
	atomic aborted;

	/*ID of this connection; unique at any point of time*/
	long id;

	/*callback of packet handle*/
	int (*handle_read)(char*);

	/*buf of recive*/
	struct buffer_queue_t *recv_queue;
	/*buf of send*/
	struct buffer_queue_t *send_queue;

	/*fd*/
	int fd;
	atomic heart_count;
	int ref;
	pthread_mutex_t ref_mutex;
	struct conn_rec_t *before;
	struct conn_rec_t *next;
}conn_rec;


conn_rec *create_conn(int fd, const char *remote_ip, int remote_port);
/*将连接对象从链表中移除*/
conn_rec *remove_connect(conn_rec *c);
/*将连接对象添加到链表*/
void add_connect(conn_rec *c);
/*释放连接对象所占用的内存空间*/
void release_connect(conn_rec *c);
void addref(conn_rec *c);
int deref(conn_rec *c);

#endif
