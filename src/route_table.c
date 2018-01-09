#include "route_table.h"

route_table t_route_table;

int init_route_table(route_table *t)
{
	t->head = NULL;
	t->max_size = 0;
	t->size = 0;
	pthread_mutex_init(&t->mutex, NULL);
}

/*头插法添加节点*/
void add_route_node(route_rec *r)
{
	pthread_mutex_lock(&t_route_table.mutex);
	if(!t_route_table.head){
		t_route_table.head = r;
		r->next = NULL;
	}else{
		r->next = t_route_table.head;
		t_route_table.head = r;
	}
	pthread_mutex_lock(&t_route_table.mutex);
}
