#include "config.h"
#include "connect.h"
#include "atomic.h"

int epfd;
int listenfd;
atomic server_stop;
apr_pool_t *server_rec;
conn_rec *conn_head = NULL;
conn_rec *conn_last= NULL;
pthread_mutex_t conn_list_mutex;
int wakeupfd;
zlog_category_t *z_cate;
