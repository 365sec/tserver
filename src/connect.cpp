#include <stdlib.h>
#include <string.h>
#include "connect.h"

struct conn_rec{
	//apr_pool *pool;

	/* client ip address*/
	char *remote_ip;
	/*client port*/
	int remote_port;

	/*are we still talking*/
	unsigned aborted:1;

	/*ID of this connection; unique at any point of time*/
	long id;

	/*callback of packet handle*/
	//int (*handle)(char*);

	/*buf of recive*/
	char buf[4096];

	/*length of had recive*/
	int buflen;

	/*fd*/
	int fd;
};

conn_rec *create_conn(int fd, const char *remote_ip, int remote_port)
{
	conn_rec *c = (conn_rec *)malloc(sizeof(conn_rec));
	c->id = 0;
	memset(c->buf, 0, sizeof(c->buf));
	c->buflen = 0;
	c->fd = fd;
	if(remote_ip){
		c->remote_ip = (char *)malloc(strlen(remote_ip)+1);
	}
	strcpy(c->remote_ip, remote_ip);
	c->remote_port = remote_port;
	return c;
}
