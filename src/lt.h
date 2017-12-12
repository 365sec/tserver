#ifndef _LT_H_
#define _LT_H_

void* epoll_loop(void *param);
int epoll_add_event(int ep, int fd, void* context);

#endif

