#include <apr_hash.h>
#include <time.h>
#include "config.h"
#include "parser_bson_data.h"


static struct _bson_parser
{
  char key[256];
  struct command_req * (*pfParse)(bson_iter_t *);
}bson_parser[]={
  {"connect", parser_bson_connect},
  {"exc_cmd_ok",parser_bson_exc_cmd_ok}
};



static apr_hash_t* bson_parse_hash = NULL;
static apr_pool_t *bson_parse_pool = NULL;

void parser_bson_init(){
	int  i;
	if(apr_pool_create(&bson_parse_pool, NULL) != APR_SUCCESS){
		 printf("内存池初始化失败!\n");
	}
	bson_parse_hash = apr_hash_make(bson_parse_pool);
	if (bson_parse_hash == NULL){
		printf("make error\n");
	}
	for(i = 0 ; i < sizeof(bson_parser)/sizeof(bson_parser[0]);i++){
	 //apr_pstrdup(bson_parse_pool,bson_parser[i].key);
	   apr_hash_set(bson_parse_hash,bson_parser[i].key,APR_HASH_KEY_STRING,&bson_parser[i]);
	}
}

struct command_rec_t * parser_bson_connect(bson_iter_t *piter){
   bool error = false ;
   bson_subtype_t    subtype;
   uint32_t uuidlen;
   const uint8_t *uuidbin;
   struct command_rec_t*  req =  command_rec_new(COMMAND_TYPE_CONNECT);

   while (bson_iter_next (piter))
   {
	   const char *key = bson_iter_key(piter);
	   if(strcmp(key,"type") == 0) {
		   if (BSON_ITER_HOLDS_INT32(piter))
		    {
		     int type= bson_iter_int32 (piter);
		     printf("type= %d \n",type);
		     req->data.connect.type = type;
		    }
	   }else if(strcmp(key,"version") == 0) {
		   if (BSON_ITER_HOLDS_UTF8(piter))
		   {
		     const char *version = bson_iter_utf8 (piter, NULL);
		     printf("version = %s\n",version);

		     req->data.connect.version = apr_pstrdup(req->pool, version);
		   }
	   }else if(strcmp(key,"sensor_id") == 0) {
		   if (BSON_ITER_HOLDS_UTF8(piter))
		   {
		 	  const char *sensor_id = bson_iter_utf8 (piter, NULL);
		 	  req->data.connect.id = apr_pstrdup(req->pool, sensor_id);
		 	  printf("id = %s\n",sensor_id);
		   }

		   /*
		 if (BSON_ITER_HOLDS_BINARY (piter))
		 {
		     bson_iter_binary (piter, &subtype, &uuidlen, &uuidbin);
		     if (subtype == BSON_SUBTYPE_UUID)
		     {
		        printf("uuid is binary");
		     }
		  }*/

	   }else if(strcmp(key,"id") == 0) {

	   }
   }
   return req;
}

struct command_rec_t * parser_bson_exc_cmd_ok(bson_iter_t *piter){
	   bool error = false ;
	   bson_subtype_t    subtype;
	   uint32_t uuidlen;
	   const uint8_t *uuidbin;
	   struct command_rec_t*  req =  command_rec_new(COMMAND_TYPE_CMD_OK);
	   while (bson_iter_next (piter))
	   {
		  const char *key = bson_iter_key(piter);
		  if(strcmp(key,"info") == 0) {
			   if (BSON_ITER_HOLDS_UTF8(piter))
			   {
			     const char *info = bson_iter_utf8 (piter, NULL);
			     req->data.exc_cmd_ok.info = apr_pstrdup(req->pool, info);
			   }
		   }
	   }
	   return req;
}


void  parse_header(bson_iter_t *piter,char* uuid,char* src,char* dest){
	   while (bson_iter_next (piter))
	   {
		   const char *key = bson_iter_key(piter);
		   if(strcmp(key,"dest") == 0)
		   {
			   if (BSON_ITER_HOLDS_UTF8(piter))
			   {
			         const char *dst = bson_iter_utf8 (piter, NULL);
			   	     strcpy(dest,dst);
			   	}
		   }else if(strcmp(key,"src") == 0)
		   {
			   if (BSON_ITER_HOLDS_UTF8(piter))
			   {
			     const char *s = bson_iter_utf8 (piter, NULL);
			     strcpy(src,s);
			   }
		   }else if(strcmp(key,"uuid") == 0)
		   {
			   if (BSON_ITER_HOLDS_UTF8(piter))
			   {
				   const char *u = bson_iter_utf8 (piter, NULL);
				   strcpy(uuid,u);
			   }
		   }
	   }
}


struct command_rec_t *parse_bson(uint8_t * my_data, size_t my_data_len){
	char* str;
	bson_t *b;
    size_t err_offset;
	bson_t bson;
	bson_iter_t iter;
	char dest[62];
	char src[62];
	char uuid[62];
	struct  command_rec_t *req = NULL;
	bool header_need = false ;
	memset(uuid,0,sizeof(uuid));
	memset(src,0,sizeof(src));
	memset(dest,0,sizeof(dest));

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
		            if (strcmp(key,"header") == 0){
		            	header_need = true;
		            	parse_header(&document_iter,uuid, src, dest);
		            }else{
						if((bp=apr_hash_get( bson_parse_hash,key,APR_HASH_KEY_STRING)  )!= NULL){
							req = bp->pfParse(&document_iter);
						}
		            }
		            bson_destroy (&b_document);
		       }
		       else
		       {
		            printf ("Bad BSON structure in key %s\n", bson_iter_key (&iter));
		       }
	        }
	    }
	    bson_destroy (&bson);
	 }

	if(req && header_need){
		req->uuid = apr_pstrdup(req->pool, uuid);
		req->src = apr_pstrdup(req->pool, src);
		req->dest = apr_pstrdup(req->pool, dest);

	}
	return req;
}

void encode_command_rep_header(bson_t * parent,struct  command_rec_t * rep){
	 bson_t child_header;
	 bson_append_document_begin (parent, "header", -1, &child_header);
     BSON_APPEND_UTF8(&child_header, "dest", rep->dest ? rep->dest : "" );
     BSON_APPEND_UTF8(&child_header, "src", rep->src ? rep->src : "" );
	 BSON_APPEND_UTF8(&child_header, "uuid", rep->uuid ? rep->uuid : "" );
	 bson_append_document_end (parent, &child_header);
}

bson_t *  encode_command_rep_to_bson (struct  command_rec_t * rep){
	  bson_t child;
	  bson_t child_header;
	  bson_t * b_object = bson_new();
	  bool error = FALSE;

	  encode_command_rep_header(b_object,rep);
	  switch (rep->type)
	  {
	    case  COMMAND_TYPE_OK:
	      bson_append_document_begin (b_object, "ok", -1, &child);
	      if(rep->data.ok.info){
	    	printf("ok_info :%s",rep->data.ok.info);
	        BSON_APPEND_UTF8(&child, "id", rep->data.ok.info);
	      }
	      bson_append_document_end (b_object, &child);
	      break;
	    case COMMAND_TYPE_CMD:
	       bson_append_document_begin (b_object, "exc_cmd", -1, &child);
	   	   if(rep->data.exc_cmd.cmdline){
	   		   	zlog_info(z_cate,"cmdline :%s",rep->data.exc_cmd.cmdline);
	   	        BSON_APPEND_UTF8(&child, "cmdline", rep->data.exc_cmd.cmdline);
	   	    }
	   	    bson_append_document_end (b_object, &child);
	   	    break;
	    default:
	    	zlog_error(z_cate,"BSON for type '%d' not implemmented", rep->type);
	        error = TRUE;
	        break;
	  }
	  if(error){
		  bson_destroy (b_object);
		  b_object = NULL;
	  }

	  return b_object;

	  /**
	   *BSON_APPEND_UTF8
	   *  case SIM_COMMAND_TYPE_NOACK:
      bson_append_document_begin (b_object, "noack", -1, &child);
      BSON_APPEND_INT32  (&child, "id", cmd->id);
      sim_uuid = sim_uuid_new_from_string (cmd->data.noack.your_sensor_id);
      bin_uuid = sim_uuid_get_uuid (sim_uuid);
      BSON_APPEND_BINARY (&child, "your_sensor_id", BSON_SUBTYPE_UUID, *bin_uuid, 16);
      g_object_unref (sim_uuid);
      bson_append_document_end (b_object, &child);
      break;*/
}


