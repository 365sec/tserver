#include<stdio.h>
#include"command.h"




struct command_rec_t* command_rec_new(command_type   type){
	struct command_rec_t* req = malloc(sizeof(struct command_rec_t));
	 req->type = type;
	 return req;

}

void command_rec_free(struct command_rec_t ** rec){
	struct command_rec_t* p = *rec;
	if(p){
		free(p);
		p = NULL;
	}

}
