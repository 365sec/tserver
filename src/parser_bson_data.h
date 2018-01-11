#include "bson.h"

#include"command.h"

void parser_bson_init();
void parser_bson_ok(bson_iter_t *piter,struct command_rec_t*  req);
struct command_rec_t * parser_bson_connect(bson_iter_t *piter);
struct command_rec_t * parser_bson_exc_cmd_ok(bson_iter_t *piter);
struct command_rec_t * parser_bson_file_upload_error(bson_iter_t *piter);
struct command_rec_t * parser_bson_file_upload_ok(bson_iter_t *piter);
struct command_rec_t *parse_bson(uint8_t * my_data, size_t my_data_len);
void  parse_header(bson_iter_t *piter,char* uuid,char* src,char* dest);
void encode_command_rep_header(bson_t * parent,struct  command_rec_t * rep);
bson_t *  encode_command_rep_to_bson (struct command_rec_t * rep);





/*
 * const uint8_t *data = bson_get_data (b);
   int towrite = b->len;
 *
 * */
