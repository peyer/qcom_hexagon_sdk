
#include <stdio.h>
#include <hexagon_standalone.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Note function4 and function8 will be garbage collected
// as they won't be used
int function1(int arg);
int function2(int arg);
int function3(int arg);
int function5(int arg);
int function6(int arg);
int function7(int arg);

static int foo(int (*myfcn_ptr)(), int arg)
{
	return myfcn_ptr(arg);
}

/*
 * function: do_dladdr
 * description:
 *   - Open a library and exercise dladdr api.
 */
static int do_dladdr()
{
    void *handle;
    char *error;
    Dl_info dli;
    void (*fun_ptr) ();
    unsigned int *addr;
	char bin_path[512] = {""};
    char *func_name = "fun_a";
    int rc;
#ifdef LINUX
	getcwd(bin_path, 511);
	strcat(bin_path, "/bin/liba.so");
#else
	strcat(bin_path, ".\\bin\\liba.so");
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
    void (*fp)(int , int ) = dlsym(handle, "fun_c");
    fp(2,3);

    dlclose(handle);
    printf("PASS - dynamic library test\n");
    return 1;
}

int foo_gc() {
  return 1;
}

/*
 * function: main
 * description:
 */
int main(int argc, char *argv[])
{

    int testpass = 0, i;
    char *builtin[]={"libgcc.so", "libc.so", "libstdc++.so"};
	int(*fPtr[6])(int) = { function1, function2, function3, function5, function6, function7 };
	printf ("\ntesting out partially linked objects\n");


	for(i=0; i<6; i++)
		printf("function%d return value = 0x%x\n", i, foo(fPtr[i], i+1));

	printf ("\n\ntesting out dynamically linked objects\n");
    dlinit(3, builtin);
    testpass = do_dladdr();
    printf ("foo_gc: %d\n", foo_gc());
    if (testpass)
        return 0;
    return 1; /* fail */
}
