#include "bson.h"

#include"command.h"

void parser_bson_init();

struct command_req * parser_bson_connect(bson_iter_t *piter);

struct command_req *parse_bson(uint8_t * my_data, size_t my_data_len);

bson_t *  encode_command_rep_to_bson (struct command_rep * rep);





/*
 * const uint8_t *data = bson_get_data (b);
   int towrite = b->len;
 *
 * */
