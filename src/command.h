/*
将网络接收的数据，转换为结构化的数据
*/
typedef enum {
  COMMAND_TYPE_NONE,
  COMMAND_TYPE_CONNECT,
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
	
	
	
};
