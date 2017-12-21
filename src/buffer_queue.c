#include <string.h>
#include "buffer_queue.h"

struct buffer_queue_t * buffer_queue_init(apr_pool_t *con_pool)
{
	struct buffer_queue_t *queue  = NULL;
	apr_pool_t *pool = NULL;
	if(apr_pool_create(&pool, con_pool) != APR_SUCCESS){
		zlog_info(z_cate, "缓存队列初始化失败!");
	  return NULL;
	}
	queue = (struct buffer_queue_t *)apr_pcalloc(pool, sizeof(struct buffer_queue_t));
	queue->pool = pool;
	queue->size = 0;
	queue->p_head = NULL;
	queue->p_last = NULL;
	return queue;
}

void buffer_queue_detroy(struct buffer_queue_t *queue)
{
	if(queue){
		if(queue->pool){
			apr_pool_destroy(queue->pool);
		}
	}
}

int buffer_queue_write(struct buffer_queue_t *queue,unsigned char *psrc,int wr_len)
{
	int writenlen = 0;
	int begin = 0;
	int retlen = 0;
    struct buffer_packet_t *packet = NULL;
    if(queue->p_last == NULL)
    {
    	packet = (struct buffer_packet_t *)apr_pcalloc(queue->pool, sizeof(struct buffer_packet_t));
    	packet->next = NULL;
    	packet->remain_size = DATA_BUFSIZE;
    	queue->p_last = queue->p_head = packet;
    }
    else
    {
        if(queue->p_last->remain_size <=0){
        	packet = (struct buffer_packet_t *)apr_pcalloc(queue->pool, sizeof(struct buffer_packet_t));
        	packet->next = NULL;
        	packet->remain_size = DATA_BUFSIZE;
        	queue->p_last->next = packet;
            queue->p_last = packet;
        }else{
        	packet = queue->p_last;
        }
    }
    /*
   if(queue->size >= queue->max_size) {
	   printf("queue size is small than max_size!\n");
       return 0;
   }
     */
   writenlen = wr_len<packet->remain_size?wr_len:packet->remain_size;
   begin = DATA_BUFSIZE-packet->remain_size;

   memcpy(packet->buffer+begin, psrc, writenlen);
   packet->remain_size -= writenlen;
   queue->size += writenlen;
   retlen+=writenlen;
   if(wr_len > writenlen){
	   retlen += buffer_queue_write(queue, psrc+writenlen, wr_len - writenlen);
   }
   return retlen;
}

int buffer_queue_write_ex(struct buffer_queue_t *dest_queue,struct buffer_queue_t *src_dest)
{
	struct buffer_packet_t   *buffer = src_dest->p_head;
	while(buffer){
		buffer_queue_write(dest_queue, buffer->buffer, DATA_BUFSIZE - buffer->remain_size);
		buffer = buffer->next;
	}
}

int buffer_queue_read(struct buffer_queue_t *queue,unsigned char *pdest,int destlen)
{
	int buffer_size = 0;
	int readlen = 0;
	int needread= 0;
    struct buffer_packet_t *packet;

    packet = queue->p_head;
	while(packet){
		buffer_size = DATA_BUFSIZE-packet->remain_size;
		needread = buffer_size<destlen-readlen?buffer_size:destlen-readlen;
		if(needread <=0){
			break;
		}
		memcpy(pdest+readlen, packet->buffer, needread);
		readlen+=needread;
		packet = packet->next;
	}
   return readlen;
}

struct buffer_packet_t *buffer_queue_pop(struct buffer_queue_t *queue)
{
    struct buffer_packet_t *packet = queue->p_head;
	if(queue->p_head){
		queue->p_head = queue->p_head->next;
		if(queue->p_head == NULL){
			queue->p_last = queue->p_head;
		}
	}
	return packet;
}

struct buffer_packet_t *buffer_queue_head(struct buffer_queue_t *queue)
{
	return queue->p_head;
}

struct buffer_packet_t *buffer_queue_last(struct buffer_queue_t *queue)
{
    struct buffer_packet_t *packet = NULL;
    if(queue->p_last == NULL)
    {
    	packet = (struct buffer_packet_t *)apr_pcalloc(queue->pool, sizeof(struct buffer_packet_t));
    	packet->next = NULL;
    	packet->remain_size = DATA_BUFSIZE;
    	queue->p_last = queue->p_head = packet;
    }else{
    	if(queue->p_last->remain_size <=0){
        	packet = (struct buffer_packet_t *)apr_pcalloc(queue->pool, sizeof(struct buffer_packet_t));
        	packet->next = NULL;
        	packet->remain_size = DATA_BUFSIZE;
    		queue->p_last->next = packet;
    		queue->p_last = packet;
    	}
    }
    return queue->p_last;
}
