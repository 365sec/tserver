#ifndef _LT_H_
#define _LT_H_
#include "connect.h"

typedef int (*handle_read_callback)(conn_rec *);

void* epoll_loop(void *param);
int create_eventfd();
void wakeup();
void handle_wake_read();
int push_complate_packet(conn_rec *c);
int handle_read(conn_rec *c);
int handle_write(conn_rec *c);
int handle_accept();
int xsend();
int is_heart_beat();
int epoll_add_event(int ep, int fd, void* context);
int packet_recv(conn_rec *c);
int sock_recv(conn_rec *c, char *recvbuf, int recvlen, int *realrecvlen);
int packet_send(conn_rec *c);
int sock_send(conn_rec *c, char *sendbuf, int sendlen, int* realsendlen);
int length_to_read(conn_rec *c);
int has_complate_packet(conn_rec *c);
int close_connect(conn_rec *c);
int s_connect(conn_rec *c);
#endif

