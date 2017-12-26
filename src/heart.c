#include "connect.h"
#include "heart.h"
#include "config.h"

void heart_handler(conn_rec *c)
{
	atomic_set(&c->heart_count, 0);
}

 void *heart_check(void *p)
{
	 zlog_info(z_cate, "心跳检测线程已启动!");

    while(!atomic_read(&server_stop)){
        check_handler();
        sleep(5);
    }
    pthread_exit((void *)1);
}

void check_handler()
{
	pthread_mutex_lock(&conn_list_mutex);
    conn_rec *c = conn_head;
    while(NULL != c  && !atomic_read(&server_stop)){
        if(atomic_read(&c->heart_count) >= 1000000){
        	zlog_info(z_cate, "客户端IP:  %s 已经掉线!", c->remote_ip);
            close(c->fd);
            c = remove_connect(c);
            continue;
        }
        else {
        	atomic_inc(&c->heart_count);
            c = c->next;
        }
    }
    pthread_mutex_unlock(&conn_list_mutex);
}
