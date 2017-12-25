#include"handler_command.h"

#include<stdio.h>
struct command_rep* handler_command_connect(struct command_req* req){

	struct command_rep * rep = command_rep_new(COMMAND_TYPE_OK);
	return rep;
}



struct command_rep* handler_command(struct command_req* req){

	struct command_rep*  rep = NULL;

	switch (req->type){

	case COMMAND_TYPE_CONNECT:
		req = handler_command_connect(req);
	default :
		return NULL;
	}
    return rep;

}
