#include <stdio.h>
#include "command.h"
#include "config.h"

struct command_rec_t* command_rec_new(command_type   type){

	apr_pool_t *pool = NULL;
	if(apr_pool_create(&pool, NULL) != APR_SUCCESS){
		zlog_error(z_cate, "command_rec创建失败!");
		return NULL;
	}
	struct command_rec_t* rec = apr_palloc(pool, sizeof(struct command_rec_t));
	memset(rec, 0, sizeof(*rec));
	rec->type = type;
	rec->pool = pool;
	return rec;

}

void command_rec_free(struct command_rec_t ** rec){
	struct command_rec_t* p = *rec;
	if(p){
		apr_pool_destroy(p->pool);
		p = NULL;
	}

}
