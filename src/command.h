#ifndef _COMMAND_H
#define _COMMAND_H

#include "apr_pools.h"

/* 数据库中下发的指令类型 */
typedef enum {
	cmdline = 0,
	file_upload,
	file_download,
	trigger
}db_command_type;

/*
将网络接收的数据，转换为结构化的数据
*/
typedef enum {
	COMMAND_TYPE_OK,
	COMMAND_TYPE_ERROR,
	COMMAND_TYPE_CONNECT,
	COMMAND_TYPE_CONNECT_OK,
	COMMAND_TYPE_CONNECT_ERROR,
	COMMAND_TYPE_CMD,
	COMMAND_TYPE_CMD_OK,
	COMMAND_TYPE_CMD_ERROR,
	COMMAND_TYPE_FILEULD,
	COMMAND_TYPE_FILEULD_OK,
	COMMAND_TYPE_FILEULD_ERROR,
	COMMAND_TYPE_FILEDLD,
	COMMAND_TYPE_FILEDLD_OK,
	COMMAND_TYPE_FILEDLD_ERROR,
 }command_type;


typedef struct command_rec_t {
	apr_pool_t *pool;
	command_type      type;   /*命令类型*/
	char*  uuid;	/*指令id*/
	char*  src;		/*源id*/
	char*  dest;	/*目标id*/
	union {
		struct {
			command_type      subtype;
			char *info;
		}ok;
		struct {
			command_type      subtype;
			char *err_info;
		}error;

		struct {
		   char* host;
		   int type;	//3 agent  2 server 1 manage
		   char* version;
		   char* id;
		}connect;

		struct {
			char* cmdline;
		}exc_cmd;

		struct {
			char *info;
		}exc_cmd_ok;
		struct {
			char *err_info;
		}exc_cmd_error;

		struct {
			char *filename; /*保存的文件名*/
			char *content;  /*文件内容*/
			int filesize;   /*文件长度*/
		}file_upload;

		struct {
			char *info;
		}file_upload_ok;

		struct {
		    char *info;
	    }file_upload_error;

	}data;
}command_rec;

struct command_rec_t*  command_rec_new(command_type   type);
void command_rec_free(struct command_rec_t** rec);

#endif



