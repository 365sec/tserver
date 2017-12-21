#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <pthread.h>

typedef struct atomic_t
{
	int i;
	pthread_mutex_t ato_mutex;
}atomic;

void atomic_init(atomic *v);
void atomic_destroy(atomic *v);
void atomic_set(atomic *v, int i);
int atomic_read(atomic *v);
/* +1 */
void atomic_inc(atomic *v);
/* -1 */
void atomic_dec(atomic *v);

#endif
