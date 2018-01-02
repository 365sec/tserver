#ifndef _COMMAND_H
#define _COMMAND_H
/*
将网络接收的数据，转换为结构化的数据
*/

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
  COMMAND_TYPE_FILEDLD,
  COMMAND_TYPE_FILEDLD_OK,
  COMMAND_TYPE_FILEDLD_ERROR,
 }command_type;


typedef struct command_rec_t {
  command_type      type;
  char*  uuid;
  char*  src;
  char*  dest;
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
       char* ok;
       char* id;
       char* sensor_ver;
       char* sensor_id;
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
   }data;
}command_rec;



struct command_rec_t*  command_rec_new(command_type   type);
void command_rec_free(struct command_rec_t** rec);

#endif



