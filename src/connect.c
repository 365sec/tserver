#include <stdlib.h>
#include <string.h>
#include "connect.h"

conn_rec *create_conn(int fd, const char *remote_ip, int remote_port)
{
	apr_pool_t *trans;
	if(apr_pool_create(&trans, server_rec) != APR_SUCCESS){
		return NULL;
	}
	conn_rec *c = (conn_rec *)apr_pcalloc(trans, sizeof(conn_rec));
	c->fd = fd;
	c->pool = trans;
	if(remote_ip){
		c->remote_ip = (char *)apr_pcalloc(trans, strlen(remote_ip)+1);
		strcpy(c->remote_ip, remote_ip);
	}
	c->remote_port = remote_port;

	c->recv_queue = buffer_queue_init(c->pool);
	c->send_queue = buffer_queue_init(c->pool);
	atomic_init(&c->heart_count);
	atomic_init(&c->aborted);
	pthread_mutex_init(&c->ref_mutex, NULL);

	context_rec *context = (context_rec *)apr_pcalloc(trans, sizeof(context_rec));
	context->id = 0;
	context->pool = trans;
	context->type = 0;
	context->version = NULL;
	context->version_id = 0;
	c->context = context;
	return c;
}

conn_rec *remove_connect(conn_rec *c)
{
	conn_rec *ret = c->next;

	if(c->before){
		c->before->next = c->next;
	}else{
		conn_head = c->next;
	}
	if(c->next) {
		c->next->before = c->before;
	}
	else{
		conn_last = c->before;
	}

	deref(c);
	return ret;
}

void release_connect(conn_rec *c)
{
	if(c->send_queue->pool){
		apr_pool_destroy(c->send_queue->pool);
	}
	if(c->recv_queue->pool){
		apr_pool_destroy(c->recv_queue->pool);
	}
	atomic_destroy(&c->heart_count);
	pthread_mutex_destroy(&c->ref_mutex);
	apr_pool_destroy(c->pool);

}

void addref(conn_rec *c)
{
	pthread_mutex_lock(&c->ref_mutex);
	c->ref++;
	pthread_mutex_unlock(&c->ref_mutex);
}

int deref(conn_rec *c)
{
	pthread_mutex_lock(&c->ref_mutex);
	c->ref--;
	if(c->ref <=0){
		release_connect(c);
		return 1;
	}
	pthread_mutex_unlock(&c->ref_mutex);
	return 0;
}

void add_connect(conn_rec *c)
{
	addref(c);
	pthread_mutex_lock(&conn_list_mutex);
	if(!conn_last){
		conn_head = conn_last = c;
		c->before = c->next = NULL;
	}else{
		conn_last->next = c;
		c->before = conn_last;
		c->next= NULL;
		conn_last = c;
	}
	pthread_mutex_unlock(&conn_list_mutex);
}


