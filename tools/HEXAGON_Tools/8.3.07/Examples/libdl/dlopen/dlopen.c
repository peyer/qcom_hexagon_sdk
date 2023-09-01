
/*
 * File: dlopen.c
 * Description:
 * 	- Tests dlopen/dlsym/dlclose commands in multi-threaded code.  
 */

#include <stdio.h>
#include <hexagon_standalone.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 0x8000

/*
 * function: thread1
 * description:
 *	- Open a library and make a call to a
 *	  function contained in it.
 */
char __attribute__ ((aligned (16))) stack1[STACK_SIZE];
void thread1()
{
    void *handle;
    void (*fun_ptr)() = NULL;
    char *error;


    handle = dlopen ("liba.so", RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        printf("thread1: dlopen error\n") ;
    }

    fun_ptr = dlsym(handle, "fun");
    if (((error = dlerror()) != NULL) ||
	(fun_ptr == NULL))
    {
        fputs(error, stderr);
    }
    else
        (*fun_ptr)();

    dlclose(handle);
 
 
   return;
}

/*
 * function: thread2
 * description:
 *	- Open a library and make a call to a
 *	  function contained in it.
 */
char __attribute__ ((aligned (16))) stack2[STACK_SIZE];
void thread2()
{
    void *handle;
    void (*fun_ptr)();
    char *error;

    handle = dlopen ("libb.so", RTLD_LAZY);
    if (!handle) {
        fputs (dlerror(), stderr);
        printf("thread2: dlopen error\n") ;
    }

    fun_ptr = dlsym(handle, "fun");
    if (((error = dlerror()) != NULL) ||
	(fun_ptr == NULL))
    {
        fputs(error, stderr);
    }
    else 
        (*fun_ptr)();

    dlclose(handle);
 
   return;
}

/*
 * function: main
 * description:
 *	- Create 2 threads and have each thread open a 
 *	  shared library.
 */
int main(int argc, char* argv[])
{
    int i;

/* 
 * dlinit initializes library structures later used
 * by the dlXX apis.
 */
    char *builtin[]={"libgcc.so", "libc.so", "libstdc++.so"};
    dlinit(3, builtin);

    thread_create(thread1, &stack1[STACK_SIZE-16], 1, NULL);
    thread_create(thread2, &stack2[STACK_SIZE-16], 2, NULL);

    /* wait for all the threads to complete. */
    for (i=1; i<=2; i++){
      printf("waiting for thread %d to finish\n", i);
      thread_join(1<<i);
    }

   printf("done.\n");
   return 0;
}
