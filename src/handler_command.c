#include <stdio.h>
#include "handler_command.h"
#include "db_process.h"
#include "zlog.h"

struct command_rec_t* handler_command_connect(struct command_rec_t* req, context_rec *ctx){
	struct command_rec_t * rep = command_rec_new(COMMAND_TYPE_OK);
    rep->data.ok.info = apr_pstrdup(rep->pool, req->data.connect.id);
    rep->data.ok.subtype = COMMAND_TYPE_CONNECT;

    pthread_mutex_lock(&ctx->context_mutex);
    ctx->version = apr_pstrdup(ctx->pool, req->data.connect.version);
    ctx->id = apr_pstrdup(ctx->pool, req->data.connect.id);
    ctx->type = req->data.connect.type;
    pthread_mutex_unlock(&ctx->context_mutex);
	return rep;
}

struct command_rec_t* handler_command_cmd_ok(struct command_rec_t* req, context_rec *ctx){

	apr_pool_t *pool = NULL;
	if(apr_pool_create(&pool, NULL) != APR_SUCCESS){
		zlog_error(z_cate, "db_exec_rec创建失败!");
		return NULL;
	}
	db_exec_rec *rec = apr_palloc(pool, sizeof(db_exec_rec));
	memset(rec, 0, sizeof(*rec));
	rec->op_id = apr_pstrdup(pool, req->uuid);
	rec->op_tye = cmdline;
	rec->op_result = apr_pstrdup(pool, req->data.exc_cmd_ok.info);
	rec->op_status = 1;
	rec->pool = pool;
	add_db_exec_rec(rec);
}


struct command_rec_t* handler_command(struct command_rec_t* req, context_rec *ctx){

	struct command_rec_t*  rep = NULL;

	switch (req->type){

	case COMMAND_TYPE_CONNECT:
		rep = handler_command_connect(req, ctx);
		break;
	case COMMAND_TYPE_CMD_OK:
		zlog_info(z_cate,"命令执行结果:%s", req->data.exc_cmd_ok.info);
		break;
	default :
		return NULL;
	}
    return rep;

}
