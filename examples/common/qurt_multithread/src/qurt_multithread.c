#include <stdio.h>
#include <stdlib.h>

#include "qurt_thread.h"
#include <qurt_barrier.h>

/* Globals */
qurt_barrier_t barrier;
short outArray[300]; /* Output from each thread will be written here */
qurt_mutex_t aMutex; /* mutex to index outArray */
short iArray; /* Index into outArray */
 
/* Function declarations */
void counter (void *);
 
/*
** Stub function to avoid linker error from the libc.a that calls this function.
** Not needed for any functionality.
*/
int sys_ftrunc(int fd, int offset)
{
   printf("test_utils.c::sys_ftrunc stubbed out\n");
   return -1;
}
int sys_opendir (char *pathname)
{
   printf("test_utils.c::sys_opendir stubbed out\n");
   return -1;
}
 
int main(int argc, char*argv[]) {
 
    qurt_thread_t tid1, tid2, tid3;
    int retcode;
    qurt_thread_attr_t attr1, attr2, attr3;
 
    /* Initialize index to 0 */
    iArray = 0;
 
    /* Set up thread 1, but don't start it yet */
    qurt_thread_attr_init(&attr1);
    qurt_thread_attr_set_name(&attr1, (char *)"cntr1");
    qurt_thread_attr_set_stack_addr(&attr1, malloc(1024));
    qurt_thread_attr_set_stack_size(&attr1, 1024);
    /*   thread priority must be set above QURT_THREAD_ATTR_PRIORITY_DEFAULT */
    qurt_thread_attr_set_priority(&attr1, QURT_THREAD_ATTR_PRIORITY_DEFAULT/2);
 
    /* Set up thread 2, but don't start it yet */
    qurt_thread_attr_init(&attr2);
    qurt_thread_attr_set_name(&attr2, (char *)"cntr2");
    qurt_thread_attr_set_stack_addr(&attr2, malloc(1024));
    qurt_thread_attr_set_stack_size(&attr2, 1024);
    /* thread priority must be set above QURT_THREAD_ATTR_PRIORITY_DEFAULT */
    qurt_thread_attr_set_priority(&attr2, QURT_THREAD_ATTR_PRIORITY_DEFAULT/2);
 
    /* Set up thread 3, but don't start it yet      */
    qurt_thread_attr_init(&attr3);
    qurt_thread_attr_set_name(&attr3, (char *)"cntr3");
    qurt_thread_attr_set_stack_addr(&attr3, malloc(4096));
    qurt_thread_attr_set_stack_size(&attr3, 4096);
    /*   thread priority must be set above QURT_THREAD_ATTR_PRIORITY_DEFAULT */
    qurt_thread_attr_set_priority(&attr3, QURT_THREAD_ATTR_PRIORITY_DEFAULT/2);
 
    /* Make sure all threads run when the gun fires */
    qurt_barrier_init(&barrier, (unsigned int)3);
 
    /* Initialize the mutex */
    qurt_mutex_init(&aMutex);
 
    /* Launch the three threads */
    retcode = qurt_thread_create(&tid1, &attr1, counter, (void *)100);
    printf("retcode %d", retcode);
    retcode = qurt_thread_create(&tid2, &attr2, counter, (void *)200);
    printf("retcode %d", retcode);
    retcode = qurt_thread_create(&tid3, &attr3, counter, (void *)300);
    printf("retcode %d", retcode);
 
    while(1) {
        /* Check value of index, if it has reached its max, then exit */
        qurt_mutex_lock(&aMutex);
        if (iArray >= 300)
            break;
        qurt_mutex_unlock(&aMutex);
    }
    qurt_mutex_unlock(&aMutex);
    for (iArray = 0; iArray < 300; iArray++)
    {
        printf("%d ", outArray[iArray]);
    }
    printf("\n");
    return 0;
}
void counter (void *val)
{
    int i = (int)val;
    qurt_barrier_wait(&barrier);
    /* Print 100 values from the start */
    while (i++ < (100 + (int)val)) {
        qurt_mutex_lock(&aMutex);
        outArray[iArray++] = i;
        qurt_mutex_unlock(&aMutex);
    }
}
