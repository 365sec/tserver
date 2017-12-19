#include "worker.h"
#include <pthread.h>

struct job_queue packet_queue;
struct job_queue result_queue;
pthread_mutex_t packet_queue_mutex;
pthread_mutex_t result_queue_mutex;
pthread_mutex_t thread_mutex;
pthread_cond_t thread_cond;
apr_pool_t *queue_pool;

void init_global_queue()
{
	packet_queue.p_head = packet_queue.p_last = NULL;
	packet_queue.max_size = 0;
	packet_queue.size = 0;
	result_queue.p_head = result_queue.p_last = NULL;
	result_queue.max_size = 0;
	result_queue.size = 0;

	 if(apr_pool_create(&queue_pool, server_rec) != APR_SUCCESS){
		 printf("队列内存池初始化失败!\n");
		 return;
	 }

	pthread_mutex_init (&packet_queue_mutex, NULL);
	pthread_mutex_init (&result_queue_mutex, NULL);
	pthread_mutex_init (&thread_mutex, NULL);
	pthread_cond_init(&thread_cond, NULL);
}

struct job_node_t *create_job_node(struct buffer_queue_t* buf_queue,struct conn_rec *c)
{
	apr_pool_t *pool;
	if(apr_pool_create(&pool, queue_pool) != APR_SUCCESS){
		return NULL;
	}
	addref(c);
	struct job_node_t *j = (struct job_node_t *)apr_pcalloc(pool, sizeof(struct job_node_t));
	j->buf_queue = buf_queue;
	j->con = c;
	j->pool = pool;
	j->before = NULL;
	j->next = NULL;
	return j;
}

void push_packet(struct buffer_queue_t *buf_queue,struct conn_rec *c)
{
	pthread_mutex_lock(&packet_queue_mutex);
	if(packet_queue.p_last == NULL){
		packet_queue.p_last = packet_queue.p_head = create_job_node(buf_queue, c);
	}else{
		struct job_node_t *j = create_job_node(buf_queue, c);
		packet_queue.p_last->next = j;
		j->before = packet_queue.p_last;
		j->next = NULL;
		packet_queue.p_last = j;
	}
	pthread_mutex_unlock(&packet_queue_mutex);
}

struct buffer_queue_t *remove_packet(struct job_node_t *j)
{
	struct job_node_t *ret = j->next;
	pthread_mutex_lock(&packet_queue_mutex);
	if(j->before){
		j->before->next = j->next;
	}
	else{
		packet_queue.p_head = j->next;
	}
	if(j->next){
		j->next->before = j->before;
	}else{
		packet_queue.p_last = j->before;
	}
	pthread_mutex_unlock(&packet_queue_mutex);

	deref(j->con);
	apr_pool_destroy(j->pool);
	return j->next;
}

struct job_node_t *first_packet()
{
	struct job_node_t *ret = NULL;
	pthread_mutex_lock(&packet_queue_mutex);
	ret = packet_queue.p_head;
	pthread_mutex_unlock(&packet_queue_mutex);
	return ret;
}
void push_result(struct buffer_queue_t *buf_queue,struct conn_rec *c)
{
	addref(c);
	pthread_mutex_lock(&result_queue_mutex);
	if(result_queue.p_last == NULL){
		result_queue.p_last = result_queue.p_head = create_job_node(buf_queue, c);
	}else{
		struct job_node_t *j = create_job_node(buf_queue, c);
		result_queue.p_last->next = j;
		j->before = result_queue.p_last;
		j->next = NULL;
		result_queue.p_last = j;
	}
	pthread_mutex_unlock(&result_queue_mutex);
}

struct buffer_queue_t *remove_result(struct job_node_t *j)
{
	struct job_node_t *ret = j->next;
	if(j->before){
		j->before->next = j->next;
	}
	else{
		result_queue.p_head = j->next;
	}
	if(j->next){
		j->next->before = j->before;
	}else{
		result_queue.p_last = j->before;
	}

	deref(j->con);
	apr_pool_destroy(j->pool);
	return j->next;
}

void *work_thread(void *p)
{
	struct job_node_t *first = NULL;
	while(1){
		pthread_mutex_lock(&thread_mutex);
		while(NULL == (first = first_packet())){
			pthread_cond_wait(&thread_cond, &thread_mutex);
		}
		pthread_mutex_unlock(&thread_mutex);
		struct buffer_queue_t *buf_queue = buffer_queue_init(first->con);

		printf("接收到一个数据包.\n");

		const char *data = "hello world!!!!!";
		buffer_queue_write(buf_queue, data, strlen(data));
		push_result(buf_queue, first->con);
		wakeup();
		remove_packet(first);
	}
}
