#include<stdio.h>
#include <time.h>
#include"util.h"


#define UUID_FORMAT "0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"


#define UUID_FORMAT_STR(uuid) ((unsigned char *)uuid)[0], ((unsigned char *)uuid)[1], ((unsigned char *)uuid)[2], ((unsigned char *)uuid)[3], \
                                  ((unsigned char *)uuid)[4], ((unsigned char *)uuid)[5], ((unsigned char *)uuid)[6], ((unsigned char *)uuid)[7], \
                                  ((unsigned char *)uuid)[8], ((unsigned char *)uuid)[9], ((unsigned char *)uuid)[10], ((unsigned char *)uuid)[11], \
                                  ((unsigned char *)uuid)[12], ((unsigned char *)uuid)[13], ((unsigned char *)uuid)[14], ((unsigned char *)uuid)[15]



int uuid_is_valid_string (const char *str)
{
  uuid_t id;
  if (uuid_parse (str, id) == 0)
    return 1;
  else
    return 0;
}


char * uuid_create_hex_string (){
	uuid_t id;
	char* hex = malloc(37);
	while ((uuid_generate_time_safe (id)) != 0)
	    usleep (10);

	uuid_unparse_lower (id, hex);
	return hex;
}

time_t t_time()
{
	return time(NULL);
}

void t_sleep_loop(int second)
{
	sleep(second);
}
