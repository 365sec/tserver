#include "config.h"
#include "connect.h"

int epfd;
int listenfd;
int server_stop = 0;
apr_pool_t *server_rec;
conn_rec *conn_head = NULL;
conn_rec *conn_last= NULL;
pthread_mutex_t conn_list_lock;
int wakeupfd;
