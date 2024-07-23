#include <pthread.h>
#include <stdio.h>
#include "atomic.h"

typedef struct spinlock {
	long locked;
	struct spinlock *next;
	char padding[48];
} mcs_spinlock_t;


void mcs_spinlock(mcs_spinlock_t **lock, mcs_spinlock_t *node)
{
	node->locked = 0;
	node->next = NULL;

	mcs_spinlock_t *prev = atomic_test_and_set(lock, node);
	if (prev == NULL) {
		return;
	}

	atomic_set(&prev->next, node);

	//spin on local node
	while (!node->locked);
}

void mcs_spinunlock(mcs_spinlock_t **lock, mcs_spinlock_t *node)
{
	mcs_spinlock_t *next = node->next;

	if (!next) {
		if (atomic_cmpxchg(lock, node, NULL) == node)
			return;
		while (!(next = node->next)) {
			//pause or relax or nop
		}
	}

	atomic_set(&next->locked, 1);
}

mcs_spinlock_t **mcs_lock_array;
mcs_spinlock_t *tail = NULL;

long sum = 0;
void *thread_fun(void *arg)
{
	mcs_spinlock_t *node = arg;
	mcs_spinlock(&tail, node);
	sum++;
	mcs_spinunlock(&tail, node);
	return (void *)0;
}

int main(int argc, char **argv)
{
	int i;
	//long sum = 0;
	pthread_t *tid_array;
	int thread_cnt = atoi(argv[1]);
	mcs_spinlock_t *node;
	tid_array = malloc(sizeof(*tid_array) * thread_cnt);
	for (i = 0; i < thread_cnt; i++) {
		node = malloc(sizeof(*node));
		pthread_create(&tid_array[i], NULL, thread_fun, (void *)node);
	}

	for (i = 0; i < thread_cnt; i++) {
		pthread_join(tid_array[i], NULL);
	}
	printf("sum of %ld threads = %d\n", thread_cnt, sum);
	
	return 0;
}
