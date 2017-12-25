#include<apr_hash.h>

#include "parser_bson_data.h"


static struct _bson_parser
{
  char key[256];
  struct command_req * (*pfParse)(bson_iter_t *);
}bson_parser[]={
  {"connect", parser_bson_connect},
};

static apr_hash_t* bson_parse_hash = NULL;
static apr_pool_t *bson_parse_pool = NULL;

void parser_bson_init(){
	int  i;
	if(apr_pool_create(&bson_parse_pool, NULL) != APR_SUCCESS){
		 printf("内存池初始化失败!\n");
	}
	printf("herer\n");
	bson_parse_hash = apr_hash_make(bson_parse_pool);
	if (bson_parse_hash == NULL){
		printf("make error\n");
	}
	for(i = 0 ; i < sizeof(bson_parser)/sizeof(bson_parser[0]);i++){
		//apr_pstrdup(bson_parse_pool,bson_parser[i].key);

	   apr_hash_set(bson_parse_hash,bson_parser[i].key,APR_HASH_KEY_STRING,&bson_parser[i]);
	}
	printf("end\n");
}


struct command_req * parser_bson_connect(bson_iter_t *piter){
   bool error = false ;
   bson_subtype_t    subtype;
   uint32_t uuidlen;
   const uint8_t *uuidbin;
   while (bson_iter_next (piter))
   {
	   const char *key = bson_iter_key(piter);
	   if(strcmp(key,"type") == 0) {
		   if (BSON_ITER_HOLDS_INT32(piter))
		    {
		     int type= bson_iter_int32 (piter);
		     printf("type= %d \n",type);
		    }
	   }else if(strcmp(key,"version") == 0) {
		   if (BSON_ITER_HOLDS_UTF8(piter))
		   {
		     const char *version = bson_iter_utf8 (piter, NULL);
		     printf("version = %s\n",version);
		   }
	   }else if(strcmp(key,"sensor_id") == 0) {
		 if (BSON_ITER_HOLDS_BINARY (piter))
		 {
		     bson_iter_binary (piter, &subtype, &uuidlen, &uuidbin);
		     if (subtype == BSON_SUBTYPE_UUID)
		     {
		        printf("uuid is binary");
		     }
		  }

	   }else if(strcmp(key,"id") == 0) {

	   }
   }
   return NULL;
}



void parse_bson(uint8_t * my_data, size_t my_data_len){
	char* str;
	bson_t *b;
    size_t err_offset;
	bson_t bson;
	bson_iter_t iter;

	if (bson_init_static (&bson, my_data, my_data_len))
	 {
	    if (bson_validate (&bson, BSON_VALIDATE_NONE, &err_offset))
	    {
	      bson_iter_init (&iter, &bson);
	      while (bson_iter_next (&iter))
	      {
	    	 char* key = NULL;
             key= bson_iter_key (&iter);

		     if (BSON_ITER_HOLDS_DOCUMENT(&iter))
		     {
		            bson_t b_document;
		            const uint8_t *document_data;
		            uint32_t document_len;
		            bson_iter_t document_iter;
		            bson_iter_document (&iter, &document_len, &document_data);
		            bson_init_static (&b_document, document_data, document_len);
		            bson_iter_init (&document_iter, &b_document);
		            struct _bson_parser *bp;
		            if((bp=apr_hash_get( bson_parse_hash,key,APR_HASH_KEY_STRING)  )!= NULL){
		            	bp->pfParse(&document_iter);
		            }
		            bson_destroy (&b_document);
		       }
		       else
		       {
		            printf ("Bad BSON structure in key %s\n", bson_iter_key (&iter));
		       }
	        }
	    }
	 }
}

