#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hexagon_standalone.h>

#define STACK_SIZE 16384
#if __HEXAGON_ARCH__ >= 4
#define COMPUTE_THREADS 3
#else
#define COMPUTE_THREADS 5 /* __HEXAGON_ARCH__ <= 3 */
#endif

// demonstrate global tls variable
__thread int tls_k=0;

static char stack [COMPUTE_THREADS][STACK_SIZE] __attribute__ ((__aligned__(8))); ;

void iterate()
{
	static __thread int tls_i=0;
	tls_i++;
	tls_k++;
	printf("Thread id: %x, tls_i=%d tls_k=%d\n", thread_get_tnum(), tls_i, tls_k+thread_get_tnum());
}

void thread_func (void* data)
{
        iterate();
		iterate();
}

int main()
{
	static __thread int tid[COMPUTE_THREADS];
	int i=0;

	thread_func(tid);

	for (i=1; i<COMPUTE_THREADS; i++)
		thread_create ((void *) thread_func,  &stack [i][STACK_SIZE], i, tid + i);

	thread_join (((1 << COMPUTE_THREADS) - 1) << 1);

	return 0;
}
