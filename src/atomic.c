#include "atomic.h"

void atomic_init(atomic *v)
{
	v->i = 0;
	pthread_mutex_init (&v->ato_mutex, NULL);
}

void atomic_destroy(atomic *v)
{
	v->i = 0;
	pthread_mutex_destroy(&v->ato_mutex);
}

void atomic_set(atomic *v, int i)
{
	pthread_mutex_lock(&v->ato_mutex);
	v->i = i;
	pthread_mutex_unlock(&v->ato_mutex);
}

int atomic_read(atomic *v)
{
	int i ;
	pthread_mutex_lock(&v->ato_mutex);
	i = v->i;
	pthread_mutex_unlock(&v->ato_mutex);
	return i;
}
/* +1 */
void atomic_inc(atomic *v)
{
	pthread_mutex_lock(&v->ato_mutex);
	v->i++;
	pthread_mutex_unlock(&v->ato_mutex);
}
/* -1 */
void atomic_dec(atomic *v)
{
	pthread_mutex_lock(&v->ato_mutex);
	v->i--;
	pthread_mutex_unlock(&v->ato_mutex);
}
