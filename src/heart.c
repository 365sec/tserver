#include "connect.h"
#include "heart.h"

void heart_handler(conn_rec *c)
{
	pthread_mutex_lock(&c->heart_mutex);
	c->heart_count = 0;
	pthread_mutex_unlock(&c->heart_mutex);
}

 void *heart_check(void *p)
{
    printf("心跳检测线程已开启!\n");

    while(!server_stop){
        check_handler();
        sleep(5);
    }
    pthread_exit((void *)1);
}

void check_handler()
{
	pthread_mutex_lock(&conn_list_lock);
    conn_rec *c = conn_head;
    while(NULL != c  && !server_stop){
    	pthread_mutex_t *mutex = &c->heart_mutex;
    	pthread_mutex_lock(mutex);
        if(c->heart_count >= 1000000){
            printf("客户端IP:  %s 已经掉线!\n", c->remote_ip);
            close(c->fd);
            c = remove_connect(c);
            pthread_mutex_unlock(mutex);
            continue;
        }
        else {
        	//printf("c->heart_count ++\n");
            c->heart_count++;
            c = c->next;
        }
        pthread_mutex_unlock(mutex);
    }
    pthread_mutex_unlock(&conn_list_lock);
}
