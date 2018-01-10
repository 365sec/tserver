#include <uuid/uuid.h>
#include "apr_pools.h"

char * uuid_create_hex_string ();
time_t t_time();
void t_sleep_loop(int second);
int t_readfile(apr_pool_t *pool, char *filename, char **buf);
