#ifndef HANDLER_COMMAND_H
#define HANDLER_COMMAND_H
#include"command.h"
#include"connect.h"


struct command_rec_t* handler_command(struct command_rec_t* req,context_rec *ctx);

#endif
