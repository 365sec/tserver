#ifndef _CONFIG_H
#define _CONFIG_H

#define SERV_PORT  5678 // 服务器端口
#define LISTENQ          128   // listen sock 参数
#define MAX_EPOLL_EVENT_COUNT      1000   // 同时监听的 epoll 数
#define EPOLL_LOOP_THREAD_COUNT  2

extern int epfd;
extern int listenfd;

#endif
