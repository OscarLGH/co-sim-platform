#include <pthread.h>
#include <stdio.h>
#include "atomic.h"

typedef struct spinlock {
	long head;
	long tail;
} spinlock_t;

void spinlock_init(spinlock_t * lock)
{
	lock->tail = lock->head = 0;
}

void spinlock(spinlock_t *lock)
{
	long old_tail;
	old_tail = atomic_xadd(&lock->tail, 1);
	while (old_tail != lock->head);
}

void spinunlock(spinlock_t *lock)
{
	atomic_inc(&lock->head);
}

spinlock_t g_spinlock;
void *thread_fun(void *arg)
{
	long *p = arg;
	spinlock(&g_spinlock);
	p[0]++;
	spinunlock(&g_spinlock);
	return (void *)0;
}

int main(int argc, char **argv)
{
	int i;
	long sum = 0;
	pthread_t *tid_array;
	int thread_cnt = atoi(argv[1]);

	spinlock_init(&g_spinlock);
	tid_array = malloc(sizeof(*tid_array) * thread_cnt);
	for (i = 0; i < thread_cnt; i++) {
		pthread_create(&tid_array[i], NULL, thread_fun, (void *)&sum);
	}

	for (i = 0; i < thread_cnt; i++) {
		pthread_join(tid_array[i], NULL);
	}
	printf("sum of %ld threads = %d\n", thread_cnt, sum);
	
	return 0;
}
