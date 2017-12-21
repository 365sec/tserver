#include <bson.h>

#include"command.h"

void parser_bson_init();

struct command_req * parser_bson_connect(bson_iter_t *piter);

void parse_bson(uint8_t * my_data, size_t my_data_len);
