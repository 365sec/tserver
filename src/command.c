#include<stdio.h>
#include"command.h"



struct command_req*  command_req_new(command_type type){
	struct command_req* req = malloc(sizeof(struct command_req));
	req->type = type;
    return req;
}


void command_free(struct command_req** req){
	struct command_req* p = *req;
	if(p){
		free(p);
		p = NULL;
	}
}



struct command_rep * command_rep_new(command_type type){
	struct command_rep* rep = malloc(sizeof(struct command_rep));
	rep->type = type;
	return rep;

}


void command_rep_free(struct command_rep** rep){
	struct command_rep* p = *rep;
	if(p){
		free(p);
		p = NULL;
	}
}
