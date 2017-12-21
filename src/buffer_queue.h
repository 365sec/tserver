#ifndef _BUFFER_QUEUE_H_
#define _BUFFER_QUEUE_H_

#include "config.h"

struct buffer_packet_t
{
    char buffer[DATA_BUFSIZE];
    int remain_size;
    //已发送长度 用于发送缓冲区
    int send_size;
    struct buffer_packet_t  *next;
};

struct buffer_queue_t
{
	apr_pool_t *pool;
    struct buffer_packet_t   *p_head,*p_last;
    /*缓冲队列最大长度(字节数)*/
    unsigned short   max_size;
    /*缓冲队列长度(字节数)*/
    unsigned short   size;
};

struct buffer_queue_t * buffer_queue_init(apr_pool_t *con_pool);
void buffer_queue_detroy(struct buffer_queue_t *p_queue);
/*函数功能：往缓冲队列中追加数据*/
int buffer_queue_write(struct buffer_queue_t *p_queue,unsigned char *p_dest,int rd_len);
int buffer_queue_write_ex(struct buffer_queue_t *dest_queue,struct buffer_queue_t *src_dest);
/*函数功能：从缓冲队列中读取所有的数据*/
int buffer_queue_read(struct buffer_queue_t *rb,unsigned char *pdest, int rd_len);
/*函数功能：从队列取出最后一个有空余数据区的块，如果不存在则创建一个*/
struct buffer_packet_t *buffer_queue_last(struct buffer_queue_t *queue);
/*函数功能：删除队列中的第一个块*/
struct buffer_packet_t *buffer_queue_pop(struct buffer_queue_t *queue);
/*函数功能：取出队列中的第一个元素*/
struct buffer_packet_t *buffer_queue_head(struct buffer_queue_t *queue);
#endif //_BUFFER_QUEUE_H_
