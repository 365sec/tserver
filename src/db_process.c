#include "db_process.h"

void *db_read_thread(void *p)
{
	long int sec1, sec2, nextcheck;
	int	cmd_num = 0, sleeptime;
	DB_CON 	con = CreateDBConnect();

	sec1 = t_time();
	sec2 = sec1;
	zlog_info(z_cate, "指令线程已启动 tid=%lu!", pthread_self());

	sleeptime = COMMANDTHREAD_TIMEOUT - sec1 % COMMANDTHREAD_TIMEOUT;
	DBconnect(con, T_DB_CONNECT_NORMAL);

	for (;;)
	{
		t_sleep_loop(sleeptime);

		sec1 = t_time();
		cmd_num = fetch_command(con);
		sec2 = t_time();

		nextcheck = sec1 - sec1 % COMMANDTHREAD_TIMEOUT + COMMANDTHREAD_TIMEOUT;
		if (0 > (sleeptime = nextcheck - sec2))
			sleeptime = 0;
	}


}

int fetch_command(DB_CON 	con)
{
	DB_RESULT	result;
	DB_ROW		row;
	result = DBselect(con, "select op_id, op_type, op_command, src_id, dest_id from opcommand");
	while (NULL != (row = DBfetch(result)))
	{
		zlog_info(z_cate, "op_id:%s, op_type:%s, op_command:%s src_id:%s, dest_id:%s", row[0], row[1], row[2], row[3], row[4]);
		struct command_rec_t *r = command_rec_new(COMMAND_TYPE_CMD);
		r->src = apr_pstrdup(r->pool, row[3]);
		r->dest = apr_pstrdup(r->pool, row[4]);
		r->uuid = apr_pstrdup(r->pool,row[0]);
		r->data.exc_cmd.cmdline = apr_pstrdup(r->pool,row[2]);
		bson_t *bs = encode_command_rep_to_bson(r);
		command_rec_free(&r);

		pthread_mutex_lock(&conn_list_mutex);
	    conn_rec *c = conn_list.conn_head;
	    while(NULL != c){
	    	pthread_mutex_lock(&c->context->context_mutex);
	    	zlog_info(z_cate,"c->context->id %s, row[0] %s", c->context->id, row[4]);
	    	if(strcmp(c->context->id, row[4])){
	    		pthread_mutex_unlock(&c->context->context_mutex);
	    		c = c->next;
	    		continue;
	    	}
	    	pthread_mutex_unlock(&c->context->context_mutex);
			if(bs){
				const uint8_t *data = bson_get_data (bs);
				int towrite = bs->len;
				struct buffer_queue_t *result_buf = buffer_queue_init(c->pool);
				buffer_queue_write(result_buf, data, towrite);
				if(result_buf){
					zlog_info(z_cate, "发送一条指令");
					push_result(result_buf, c);
					wakeup();
				}
			}
	    	c = c->next;
	    }
	    if(bs){
	    	bson_destroy(bs);
	    }
	    pthread_mutex_unlock(&conn_list_mutex);
	}
	DBfree_result(result);
}

db_exec_list t_db_exec_list;
pthread_cond_t t_db_exec_cond;

int init_db_exec_list()
{
	t_db_exec_list.head = NULL;
	t_db_exec_list.max_size = 0;
	t_db_exec_list.size = 0;
	pthread_mutex_init(&t_db_exec_list.mutex, NULL);
	pthread_cond_init(&t_db_exec_cond, NULL);
}

/*头插法添加节点*/
void add_db_exec_rec(db_exec_rec *r)
{
	pthread_mutex_lock(&t_db_exec_list.mutex);
	if(!t_db_exec_list.head){
		t_db_exec_list.head = r;
		r->next = NULL;
	}else{
		r->next = t_db_exec_list.head;
		t_db_exec_list.head = r;
	}
	pthread_mutex_lock(&t_db_exec_list.mutex);
	pthread_cond_signal(&t_db_exec_cond);
}

db_exec_rec *get_db_exec_rec()
{
	db_exec_rec *r = NULL;
	if(t_db_exec_list.head){
		r = t_db_exec_list.head;
		t_db_exec_list.head = r->next;
	}
	return r;
}

void *db_write_thread(void *p)
{
	db_exec_rec *r = NULL;
	DB_CON 	con = CreateDBConnect();
	zlog_info(z_cate, "数据库线程已启动 tid=%lu!", pthread_self());
	DBconnect(con, T_DB_CONNECT_NORMAL);

	for(;;){
		pthread_mutex_lock(&t_db_exec_list.mutex);
		while(NULL == (r = get_db_exec_rec())){
			pthread_cond_wait(&t_db_exec_cond, &t_db_exec_list.mutex);
			zlog_info(z_cate, "pthread_cond_wait!");
		}
		pthread_mutex_unlock(&t_db_exec_list.mutex);
		DBexecute(con, "insert into opresult (op_id, op_result, op_type, op_status)"
			" values ('%d', '%s', '%d', '%d');", r->op_id, r->op_result, r->op_tye, r->op_status);
		apr_pool_destroy(r->pool);
	}

}
