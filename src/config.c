
#include "config.h"
#include "connect.h"
#include "atomic.h"

int epfd;
int listenfd;
int server_stop;
apr_pool_t *server_rec;
conn_rec_list conn_list;
pthread_mutex_t conn_list_mutex;
int wakeupfd;
zlog_category_t *z_cate;
int c_fd = 0;
char *db_host = "172.16.39.95";
char *db_user = "root";
char *db_password = "";
char *db_dbname = "cmd_system";
char *db_schema = NULL;
char *db_socket = NULL;
int db_port = 3306;

