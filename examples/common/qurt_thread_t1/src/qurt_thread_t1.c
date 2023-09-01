#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <qurt.h>
//#include "profile.h"

#define THREAD_NUM 64
#define STACK_SIZE 1024
#define MAX_ITERATION 1024

qurt_barrier_t testbarrier;
typedef struct thread_info_type {
    qurt_thread_t id;
    char name[10];
    unsigned prio;
    void* stack_addr;
} thread_info_t;
thread_info_t thread_info[THREAD_NUM];

static void testmain(void * argv)
{
    int status, id, prio;
    char name[10] = { 0 };
    thread_info_t* pthread_info = argv;
    
    qurt_barrier_wait(&testbarrier);
    qurt_thread_get_name(name, 10);
    status = strcmp(pthread_info->name, name);
    assert(status == 0);
    printf("%s starts\n", name);
    
    id = qurt_thread_get_id();
    assert(id == pthread_info->id);
    prio = qurt_thread_get_priority(id);
    assert(prio == pthread_info->prio);
    
    printf("%s: exiting\n", name);
        
    qurt_thread_exit(id);
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
    int i, status, thread_ret;

    printf("In Main");
    qurt_thread_attr_init(&thread_attr);
    qurt_barrier_init(&testbarrier, THREAD_NUM+1);
    
    for (i=0; i<THREAD_NUM; i++) {
        thread_info[i].stack_addr=malloc(STACK_SIZE);
        assert(thread_info[i].stack_addr!=NULL);
        snprintf(thread_info[i].name, 10, "thread%d", i);
        qurt_thread_attr_set_name(&thread_attr,thread_info[i].name);
        qurt_thread_attr_set_stack_addr(&thread_attr, thread_info[i].stack_addr);
        qurt_thread_attr_set_stack_size(&thread_attr, STACK_SIZE);
        thread_info[i].prio = 100+i;
        qurt_thread_attr_set_priority(&thread_attr,thread_info[i].prio);
        status=qurt_thread_create(&thread_info[i].id, &thread_attr, testmain, &thread_info[i]);
        printf("thread%d created id = 0x%x\n", i, thread_info[i].id);
        assert(QURT_EOK==status);
    }
    
    qurt_barrier_wait(&testbarrier);

    for (i=0; i<THREAD_NUM; i++) {
        status = qurt_thread_join(thread_info[i].id, &thread_ret);
        assert(status == QURT_EOK || status == QURT_ENOTHREAD);
        printf("thread%d return status 0x%x\n", i, thread_ret);
        if (status == QURT_EOK )
            assert(thread_ret== thread_info[i].id);
        free(thread_info[i].stack_addr);
    }   
    
    qurt_barrier_destroy(&testbarrier);
    
    //DECLARE_COVERED_APIS("qurt_thread_attr_init qurt_thread_attr_set_name qurt_thread_attr_set_stack_addr qurt_thread_attr_set_stack_size qurt_thread_attr_set_priority");
    // DECLARE_COVERED_APIS("qurt_thread_create qurt_thread_join qurt_thread_exit qurt_thread_get_name qurt_thread_get_id qurt_thread_get_priority");
    return 0;
}

