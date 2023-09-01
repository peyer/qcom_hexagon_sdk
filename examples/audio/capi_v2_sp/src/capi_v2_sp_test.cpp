#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>

#include "test_main.h"
#include "test_capi_v2.h"

#include "capi_v2_sp_tx.h"
#include "capi_v2_sp_rx.h"


#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

#define TRY(exception, func) \
	if (TEST_SUCCESS != (exception = func)) {\
		goto exception##bail; \
	}

#define THROW(exception, errno) \
	exception = errno; \
goto exception##bail;

#define CATCH(exception) exception##bail: if (exception != TEST_SUCCESS)


typedef struct
{
	char* input_rx_filename;
	char* output_rx_filename;
	char* config_rx_filename;
	char* input_tx_filename;
	char* output_tx_filename;
	char* config_tx_filename;
}
args_t;

void get_eargs(
		int32_t argc,
		char* argv[],
		args_t* input_args
	      );

#ifdef __V_DYNAMIC__
int dll_test(args_t* input_args, capi_v2_proplist_t* init_set_properties,
		capi_v2_proplist_t* static_properties)
{
	void* h = 0;
	//int (*pfn)(void) = 0;
	const char* cpszmodname = "capi_v2_sp.so";
	const char* cpszget_tx_static_properties = "capi_v2_sp_tx_get_static_properties";
	const char* cpszinit_tx = "capi_v2_sp_tx_init";

	const char* cpszget_rx_static_properties = "capi_v2_sp_rx_get_static_properties";
	const char* cpszinit_rx = "capi_v2_sp_rx_init";

	capi_v2_get_static_properties_f get_tx_static_properties_f = 0;
	capi_v2_init_f init_tx_f = 0;

	capi_v2_get_static_properties_f get_rx_static_properties_f = 0;
	capi_v2_init_f init_rx_f = 0;

	int err = TEST_SUCCESS;

	FARF(HIGH, "-- start dll test --                                                ");

	FARF(HIGH, "attempt to load   %s                               ", cpszmodname);
	h = dlopen(cpszmodname, RTLD_NOW);
	if (0 == h)   {
		FARF(HIGH, "dlopen %s failed %s                           ", cpszmodname, dlerror());
		THROW(err, TEST_FAILURE);
	}
	get_tx_static_properties_f = (capi_v2_get_static_properties_f)dlsym(h, cpszget_tx_static_properties);
	if (0 == get_tx_static_properties_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszget_tx_static_properties, dlerror());
		THROW(err, TEST_FAILURE);
	}
	init_tx_f = (capi_v2_init_f)dlsym(h, cpszinit_tx);
	if (0 == init_tx_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszinit_tx, dlerror());
		THROW(err, TEST_FAILURE);
	}

	get_rx_static_properties_f = (capi_v2_get_static_properties_f)dlsym(h, cpszget_rx_static_properties);
	if (0 == get_rx_static_properties_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszget_rx_static_properties, dlerror());
		THROW(err, TEST_FAILURE);
	}
	init_rx_f = (capi_v2_init_f)dlsym(h, cpszinit_rx);
	if (0 == init_rx_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszinit_rx, dlerror());
		THROW(err, TEST_FAILURE);
	}


#ifdef ENABLE_COMMAND_LINE_PARAMS
	TRY(err,
			test_capi_v2_main_feedback(get_rx_static_properties_f, init_rx_f,
				get_tx_static_properties_f, init_tx_f,
				init_set_properties, static_properties,
				input_args->input_rx_filename,
				input_args->output_rx_filename,
				input_args->config_rx_filename,
				input_args->input_tx_filename,
				input_args->output_tx_filename,
				input_args->config_tx_filename));

#else 
	TRY(err,
		test_capi_v2_main_feedback(get_rx_static_properties_f, init_rx_f,
				get_tx_static_properties_f, init_tx_f,
				init_set_properties, static_properties,
				"../data/input_rx.raw",
				"../data/output_rx.raw",
				"../data/sp_rx.cfg",
				"../data/input_tx.raw",
				"../data/output_tx.raw",
				"../data/sp_tx.cfg"));

#endif	

	FARF(HIGH, "closing %s                                                 ", cpszmodname);
	dlclose(h);
	h = 0;

	FARF(HIGH, "Test Passed                                                         ");

	CATCH(err){};
	if (0 != h) {
		dlclose(h);
	}
	FARF(HIGH, "-- end dll test --                                                  ");
	return err;
}
#else
int lib_test(args_t* input_args, capi_v2_proplist_t* init_set_properties,
		capi_v2_proplist_t* static_properties)
{

	int err = TEST_SUCCESS;

	FARF(HIGH, "-- start lib test --                                                ");

#ifdef ENABLE_COMMAND_LINE_PARAMS
	TRY(err,
			test_capi_v2_main_feedback(capi_v2_sp_rx_get_static_properties, capi_v2_sp_rx_init,
				capi_v2_sp_tx_get_static_properties, capi_v2_sp_tx_init,
				init_set_properties, static_properties,
				input_args->input_rx_filename,
				input_args->output_rx_filename,
				input_args->config_rx_filename,
				input_args->input_tx_filename,
				input_args->output_tx_filename,
				input_args->config_tx_filename));

#else 
	TRY(err,
		test_capi_v2_main_feedback(capi_v2_sp_rx_get_static_properties, capi_v2_sp_rx_init,
				capi_v2_sp_tx_get_static_properties, capi_v2_sp_tx_init,
				init_set_properties, static_properties,
				"../data/input_rx.raw",
				"../data/output_rx.raw",
				"../data/sp_rx.cfg",
				"../data/input_tx.raw",
				"../data/output_tx.raw",
				"../data/sp_tx.cfg"));

#endif	
	FARF(HIGH, "Test Passed                                                         ");

	CATCH(err){};
	FARF(HIGH, "-- end lib test test --                                             ");
	return err;
}
#endif

/*------------------------------------------------------------------------
  Function name: get_args
  Description- Read input arguments
 * -----------------------------------------------------------------------*/
void get_eargs(
		int32_t argc,
		char* argv[],
		args_t* input_args
	      )
{
	int16_t input_option;

	if (argc < 12) {
		// usage(stdout, argv[0]);
		fprintf(stderr, "%s: argc < 12.\n", argv[0]);
		exit(-1);
	} else {
		input_args->input_rx_filename = NULL;
		input_args->output_rx_filename = NULL;
		input_args->config_rx_filename = NULL;
		input_args->input_tx_filename = NULL;
		input_args->config_tx_filename = NULL;

		while ((input_option = getopt(argc, argv, "i:o:c:d:e:f:")) != -1) {
			switch(input_option) {
				case 'i':
					input_args->input_rx_filename = optarg;
					break;

				case 'o':
					input_args->output_rx_filename = optarg;
					break;

				case 'c':
					input_args->config_rx_filename = optarg;
					break;

				case 'd':
					input_args->input_tx_filename = optarg;
					break;

				case 'e':
					input_args->output_tx_filename = optarg;
					break;

				case 'f':
					input_args->config_tx_filename = optarg;
					break;
			}
		}
	}
}

int test_main_start(int argc, char* argv[])
{
	int err = TEST_SUCCESS;
	args_t* input_args = 0;

	if (NULL == (input_args = (args_t*)malloc(sizeof(args_t)))) {
		FARF(HIGH, "%s:  ERROR CODE 1 - Cannot malloc args\n", argv[0]);
		exit(-1);
	}

#ifdef ENABLE_COMMAND_LINE_PARAMS
	get_eargs(argc, argv, input_args);
#endif
	capi_v2_proplist_t init_set_properties;
	capi_v2_proplist_t static_properties;
	capi_v2_prop_t props[6];
	capi_v2_init_memory_requirement_t init_memory_requirement;
	capi_v2_stack_size_t stack_size;
	capi_v2_is_inplace_t is_inplace;
	capi_v2_requires_data_buffering_t requires_data_buffering;
	capi_v2_num_needed_framework_extensions_t num_needed_framework_extensions;
	int8_t i = 0;

	props[i].id = CAPI_V2_INIT_MEMORY_REQUIREMENT;
	props[i].payload.data_ptr = (int8_t*)&init_memory_requirement;
	props[i].payload.max_data_len = sizeof(init_memory_requirement);
	i++;

	props[i].id = CAPI_V2_STACK_SIZE;
	props[i].payload.data_ptr = (int8_t*)&stack_size;
	props[i].payload.max_data_len = sizeof(stack_size);
	i++;

	props[i].id = CAPI_V2_IS_INPLACE;
	props[i].payload.data_ptr = (int8_t*)&is_inplace;
	props[i].payload.max_data_len = sizeof(is_inplace);
	i++;

	props[i].id = CAPI_V2_REQUIRES_DATA_BUFFERING;
	props[i].payload.data_ptr = (int8_t*)&requires_data_buffering;
	props[i].payload.max_data_len = sizeof(requires_data_buffering);
	i++;

	props[i].id = CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS;
	props[i].payload.data_ptr = (int8_t*)&num_needed_framework_extensions;
	props[i].payload.max_data_len = sizeof(num_needed_framework_extensions);
	i++;

	init_set_properties.props_num = 0;
	static_properties.props_num = i;
	static_properties.prop_ptr = props;


#ifdef __V_DYNAMIC__
	TRY(err, dll_test(input_args, &init_set_properties, &static_properties));
#else
	TRY(err, lib_test(input_args, &init_set_properties, &static_properties));
#endif

	if (0 == input_args) {
		free(input_args);
	}

	CATCH(err){};

	return err;
}

