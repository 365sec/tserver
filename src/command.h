#ifndef _COMMAND_H
#define _COMMAND_H
/*
将网络接收的数据，转换为结构化的数据
*/
typedef enum {
  COMMAND_TYPE_NONE,
  COMMAND_TYPE_CONNECT,
  COMMAND_TYPE_OK,
  COMMAND_TYPE_PING,
 }command_type;


struct command_req {
  command_type      type;
  union {
    struct {
       char* host;
       char* ok;
       char* id;
       char* sensor_ver;
       char* sensor_id;
    }connect;
   }data;
};

struct command_rep{
	command_type      type;
	union {
	    struct {
	       char* ok;
	    }connect_rep_ok;

	    struct {
	 	   char* ok;
	 	}connect_rep_failed;

	 }data;
};



struct command_req*  command_req_new(command_type   type);
void command_req_free(struct command_req** req);


struct command_rep * command_rep_new(command_type type);
void command_rep_free(struct command_rep** req);

#endif
