#include"handler_command.h"

#include<stdio.h>
struct command_rec_t* handler_command_connect(struct command_rec_t* req){

	struct command_rec_t * rep = command_rec_new(COMMAND_TYPE_OK);
	return rep;
}



struct command_rec_t* handler_command(struct command_rec_t* req){

	struct command_rec_t*  rep = NULL;

	switch (req->type){

	case COMMAND_TYPE_CONNECT:
		rep = handler_command_connect(req);
		break;
	default :
		return NULL;
	}
    return rep;

}
