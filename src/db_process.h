#ifndef _DB_PROCESS_H
#define _DB_PROCESS_H

#include "tdb.h"
#include "command.h"
#include "parser_bson_data.h"
#include "connect.h"

typedef struct db_exec_rec_t{
	apr_pool_t *pool;
	char *op_id;			/*命令id*/
	char *op_result;    	/*命令执行结果*/
	int op_tye;			/*命令类型 如系统命令 文件操作等*/
	int op_status;			/*命令执行的状态 0 成功 1 失败 2未完成*/
	struct db_exec_rec_t *next;
}db_exec_rec;

typedef struct db_exec_list_t
{
	pthread_mutex_t mutex;
	db_exec_rec *head;
	int max_size;
	int size;
}db_exec_list;

void *db_read_thread(void *p);
int fetch_command(DB_CON 	con);
void *db_write_thread(void *p);
void add_db_exec_rec(db_exec_rec *r);
int init_db_exec_list();

#endif
