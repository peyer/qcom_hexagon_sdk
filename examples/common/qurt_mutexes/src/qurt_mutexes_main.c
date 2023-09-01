#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <qurt.h>
// #include "profile.h"

#define THREAD_NUM 8
#define STACK_SIZE 1024

/*verify lowest prio thread get mutex when multiple threads waiting*/

qurt_mutex_t testmutex;
qurt_anysignal_t testsignal;

static void testmain(void * argv)
{
    static int nTestVal1 = -1, nTestVal2 = -1;
    int j, id_index; //status
    unsigned int nIteration=0;
    char thread_name[10];
    static unsigned int access_order;

    qurt_thread_get_name(thread_name, 10);
    //assert(status==QURT_EOK); TODO check user guide
    //printf("%s starts\n", thread_name);
    id_index = atoi(&thread_name[6]);
    printf("thread%d starts\n", id_index);

    qurt_mutex_lock( &testmutex );
    printf("\n%s Mutex lock acquired, Access order = %d\n", thread_name, access_order);
    if (id_index == 0) {
        qurt_anysignal_set(&testsignal, 0x1);
        assert(access_order == 0);
    }
    else {
        assert(access_order == (THREAD_NUM-id_index));
    }
    access_order++;
    while (++nIteration<=128)
    {
        printf(".");
        nTestVal1 = -1;
        nTestVal2 = -1;
        for (j=0; j<1000; j++)
        {
            nTestVal1++;
            nTestVal2++;
            assert(nTestVal1 == j&&nTestVal2 == j);
        }
        //printf("%s: verified first lock succeed\n", thread_name);
    }
    qurt_mutex_unlock( &testmutex );
    printf("\n%s: verified pass\n", thread_name);
        
    qurt_thread_exit(QURT_EOK);
}

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

int main(int argc, char **argv)
{


    qurt_thread_attr_t thread_attr;
    char* thread_stack[THREAD_NUM];
    qurt_thread_t thread_id[THREAD_NUM];
    int i=0, status;
    char thread_name[10];

    qurt_thread_attr_init(&thread_attr);
    qurt_mutex_init(&testmutex);
    qurt_anysignal_init(&testsignal);
    
    // create the lowest prio thread
    thread_stack[0]=malloc(STACK_SIZE);
    assert(thread_stack[0]!=NULL);
    snprintf(thread_name, 10, "thread0");
    qurt_thread_attr_set_name(&thread_attr,thread_name);
    qurt_thread_attr_set_stack_addr(&thread_attr, thread_stack[i]);
    qurt_thread_attr_set_stack_size(&thread_attr, STACK_SIZE);
    qurt_thread_attr_set_priority(&thread_attr,100);
    status=qurt_thread_create(&thread_id[0], &thread_attr, testmain, NULL);
    assert(QURT_EOK==status);
    
    qurt_anysignal_wait(&testsignal, 0x1); // wait until thread0 is in CS
    for (i=1; i<THREAD_NUM; i++) {
        thread_stack[i]=malloc(STACK_SIZE);
        assert(thread_stack[i]!=NULL);
        
        snprintf(thread_name, 10, "thread%d", i);
        qurt_thread_attr_set_name(&thread_attr,thread_name);
        qurt_thread_attr_set_stack_addr(&thread_attr, thread_stack[i]);
        qurt_thread_attr_set_stack_size(&thread_attr, STACK_SIZE);
        qurt_thread_attr_set_priority(&thread_attr,100-i);
        status=qurt_thread_create(&thread_id[i], &thread_attr, testmain, NULL);
        assert(QURT_EOK==status);
    }

    for (i=0; i<THREAD_NUM; i++) {
        qurt_thread_join(thread_id[i], &status);
        assert(status == QURT_EOK);
        printf("thread%d return EOK\n", i);
    }   
    
    qurt_mutex_destroy(&testmutex);
    qurt_anysignal_destroy(&testsignal);
    
    //DECLARE_COVERED_APIS("qurt_mutex_init qurt_mutex_lock qurt_mutex_unlock qurt_mutex_destroy");
    return 0;
}

