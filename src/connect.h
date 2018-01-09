#ifndef _CONNECT_H_
#define _CONNECT_H_

#include "buffer_queue.h"
#include "config.h"
#include "atomic.h"

typedef struct context_rec_t{
	/* pool of connect_rec */
	apr_pool_t *pool;
	/*ID of this connection; unique at any point of time*/
	char *id;
	/* client type, manage or agent */
	int type;
	/* client version */
	char *version;
	/* lock of the context*/
	pthread_mutex_t context_mutex;
}context_rec;

typedef struct conn_rec_t{
	/*pool of connect_rec*/
	apr_pool_t *pool;

	/* client ip address*/
	char *remote_ip;
	/*client port*/
	int remote_port;

	/*are we still talking*/
	atomic aborted;

	/*read callback of packet handle*/
	int (*read_callback)(struct conn_rec_t *c);
	/*close callback of packet handle*/
	int (*close_callback)(struct conn_rec_t *c);

	/*buf of recive*/
	struct buffer_queue_t *recv_queue;
	/*buf of send*/
	struct buffer_queue_t *send_queue;

	/*fd*/
	int fd;
	/*heartbeat count*/
	atomic heart_count;
	/*ref count*/
	int ref;
	pthread_mutex_t ref_mutex;
	/*the context which is pass the handle_command*/
	context_rec *context;
	/*use for the conn_rec list*/
	struct conn_rec_t *before;
	struct conn_rec_t *next;
}conn_rec;

typedef struct conn_rec_list_t
{
	conn_rec *conn_head;
	conn_rec *conn_last;
	int size;
}conn_rec_list;

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
