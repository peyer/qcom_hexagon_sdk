#include <stdio.h>
#include <hexagon_standalone.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

void *handle;
int (*fun_ptr) ();

static int foo(int (*myfcn_ptr)(), int arg)
{
	return myfcn_ptr(arg);
}

static int do_dladdr()
{
    //void *handle;
    char *error;
    Dl_info dli;
    //void (*fun_ptr) ();
    unsigned int *addr;
    char *func_name = "function1";
	char bin_path[512] = {""};
    int rc;

#ifdef LINUX
	getcwd(bin_path, 511);
	strcat(bin_path, "/bin/mylib1.so");
#else
	strcat(bin_path, ".\\bin\\mylib1.so");
#endif

    handle = dlopen(bin_path, RTLD_LAZY);
    if (!handle)
    {
		fputs(dlerror(), stderr);
		printf("\nglob_object: dlopen error\n");
		return 0;
    }

    rc = dladdr(0, 0);
    if (rc != 0)
    {
		printf("FAIL: dladdr (0,0) rc = %d\n", rc);
		fprintf(stderr, "FAIL dladdr got %d expected 0\n", rc);
		return 0;
    }

    fun_ptr = dlsym(handle, func_name);

    rc = dladdr((void *)fun_ptr, &dli);

    if (rc == 0)
    {
		printf("rc = %d\n", rc);
		fprintf(stderr, "dladdr failed\n");
		return 0;
    }

    printf("dli_fname = %s\n", dli.dli_fname);
    printf("dli_fbase = 0x%p\n", dli.dli_fbase);
    printf("dli_sname = %s\n", dli.dli_sname);
    printf("dli_saddr = 0x%p\n", dli.dli_saddr);

    if (strcmp(dli.dli_sname, func_name))
    {
		printf("FAIL: mismatch in expected function name\n");
		printf("\n expected %s, got %s\n", func_name, dli.dli_sname);
		return 0;
    }
	// clear error messages if you've made it this far
	dlerror();

    return 1;
}

int foo_gc() {
  return 1;
}


int main(int argc, char** argv)
{


    int i,  testpass = 0;;
    char *error=0;
	char *fNameStr[3] = {"function1", "function2", "function3"};
	char bin_path[256] = {""};
    char *builtin[]={"libgcc.so", "libc.so", "libstdc++.so"};
	// initialize
    dlinit(3, builtin);
	// open shared library
    testpass = do_dladdr();

/*
 * Call functions from shared library
*/

	for(i=0; i<3; i++)
	{
		fun_ptr = dlsym(handle, fNameStr[i]);
		if (((error = dlerror()) != NULL) ||
		(fun_ptr == NULL))
		{
			fputs(error, stderr);
			return -1;
		}
		printf("%s return value = %x\n", fNameStr[i], foo(fun_ptr, i+1));
	}

    dlclose(handle);
    if (testpass)
	{
		printf("PASS - dynamic library test\n");
		return 0;
	}

    return 1; /* fail */
}