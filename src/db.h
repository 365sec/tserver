#ifndef _DB_H
#define _DB_H

#include "config.h"

#define T_DB_DOWN 2   //数据库down
#define T_DB_FAIL 1   //数据库错误
#define T_DB_OK 0
//数据库连接选项
#define T_DB_CONNECT_ONCE   0   //仅连接一次
#define T_DB_CONNECT_NORMAL 1   //正常连接
#define T_DB_CONNECT_EXIT   2   //连接后直接退出

#define T_DB_WAIT_DOWN  3    //重连等待时间(秒)

#define SUCCEED 0
#define FAILED 1

//存储fetch后的单行记录
typedef char	**DB_ROW;
//存储结果集
typedef struct db_result_t	*DB_RESULT;
typedef struct db_conn_t	*DB_CON;

int	t_db_init(const char *dbname);
void t_db_close(struct db_conn_t *con);
int	t_db_connect(struct db_conn_t *con,char *host, char *user, char *password, char *dbname, char *dbschema, char *dbsocket, int port);
int	t_db_begin(struct db_conn_t *con);
int	t_db_commit(struct db_conn_t *con);
int	t_db_rollback(struct db_conn_t *con);
DB_RESULT	t_db_vselect(struct db_conn_t *con, const char *fmt, va_list args);
int	t_db_vexecute(struct db_conn_t *con, const char *fmt, va_list args);
DB_ROW	t_db_fetch(DB_RESULT result);
void DBfree_result(DB_RESULT result);
int	zbx_db_txn_level(void);
int	zbx_db_txn_error(void);
struct db_conn_t *create_db_connect();
void release_db_connect(struct db_conn_t *con);

#endif

