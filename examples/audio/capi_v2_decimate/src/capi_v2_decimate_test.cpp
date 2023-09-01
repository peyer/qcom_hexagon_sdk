#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>

#include "test_main.h"
#include "test_capi_v2.h"

#include "capi_v2_decimate.h"

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
	char* input_filename;
	char* output_filename;
	char* config_filename;
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
	const char* cpszmodname = "capi_v2_decimate.so";
	const char* cpszget_static_properties = "capi_v2_decimate_get_static_properties";
	const char* cpszinit = "capi_v2_decimate_init";

	capi_v2_get_static_properties_f get_static_properties_f = 0;
	capi_v2_init_f init_f = 0;
	int err = TEST_SUCCESS;

	FARF(HIGH, "-- start dll test --                                                ");

	FARF(HIGH, "attempt to load   %s                               ", cpszmodname);
	h = dlopen(cpszmodname, RTLD_NOW);
	if (0 == h)   {
		FARF(HIGH, "dlopen %s failed %s                           ", cpszmodname, dlerror());
		THROW(err, TEST_FAILURE);
	}
	get_static_properties_f = (capi_v2_get_static_properties_f)dlsym(h, cpszget_static_properties);
	if (0 == get_static_properties_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszget_static_properties, dlerror());
		THROW(err, TEST_FAILURE);
	}
	init_f = (capi_v2_init_f)dlsym(h, cpszinit);
	if (0 == init_f)   {
		FARF(HIGH, "dlsym %s failed %s                              ", cpszinit, dlerror());
		THROW(err, TEST_FAILURE);
	}

#ifdef ENABLE_COMMAND_LINE_PARAMS
	TRY(err, test_capi_v2_main(get_static_properties_f,
			init_f,
			init_set_properties,
			static_properties,
			input_args->input_filename,
			input_args->output_filename,
			input_args->config_filename));
#else

	TRY(err, test_capi_v2_main(get_static_properties_f,
			init_f,
			init_set_properties,
			static_properties,
			"../data/input.raw",
			"../data/output_decimation_factor_set.raw",
			"../data/decimation_factor_set.cfg"));

	TRY(err, test_capi_v2_main(get_static_properties_f,
			init_f,
			init_set_properties,
			static_properties,
			"../data/input.raw",
			"../data/output_decimate_enable.raw",
			"../data/decimate_enable.cfg"));
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
	TRY(err, test_capi_v2_main(capi_v2_decimate_get_static_properties,
			capi_v2_decimate_init,
			init_set_properties,
			static_properties,
			input_args->input_filename,
			input_args->output_filename,
			input_args->config_filename));
#else

	TRY(err, test_capi_v2_main(capi_v2_decimate_get_static_properties,
			capi_v2_decimate_init,
			init_set_properties,
			static_properties,
			"../data/input.raw",
			"../data/output_decimation_factor_set.raw",
			"../data/decimation_factor_set.cfg"));

	TRY(err, test_capi_v2_main(capi_v2_decimate_get_static_properties,
			capi_v2_decimate_init,
			init_set_properties,
			static_properties,
			"../data/input.raw",
			"../data/output_decimate_enable.raw",
			"../data/decimate_enable.cfg"));
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

	if (argc < 6) {
		// usage(stdout, argv[0]);
		fprintf(stderr, "%s: argc < 6.\n", argv[0]);
		exit(-1);
	} else {
		input_args->input_filename = NULL;
		input_args->output_filename = NULL;
		input_args->config_filename = NULL;
		while ((input_option = getopt(argc, argv, "i:o:c:")) != -1) {
			switch(input_option) {
			case 'i':
				input_args->input_filename = optarg;
				break;

			case 'o':
				input_args->output_filename = optarg;
				break;

			case 'c':
				input_args->config_filename = optarg;
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
	capi_v2_max_metadata_size_t max_metadata_size;
	capi_v2_is_inplace_t is_inplace;
	capi_v2_requires_data_buffering_t requires_data_buffering;
	capi_v2_num_needed_framework_extensions_t num_needed_framework_extensions;

	init_set_properties.props_num = 0;
	static_properties.props_num = 6;
	static_properties.prop_ptr = props;

	props[0].id = CAPI_V2_INIT_MEMORY_REQUIREMENT;
	props[0].payload.data_ptr = (int8_t*)&init_memory_requirement;
	props[0].payload.max_data_len = sizeof(init_memory_requirement);

	props[1].id = CAPI_V2_STACK_SIZE;
	props[1].payload.data_ptr = (int8_t*)&stack_size;
	props[1].payload.max_data_len = sizeof(stack_size);

	props[2].id = CAPI_V2_MAX_METADATA_SIZE;
	props[2].payload.data_ptr = (int8_t*)&max_metadata_size;
	props[2].payload.max_data_len = sizeof(max_metadata_size);

	props[3].id = CAPI_V2_IS_INPLACE;
	props[3].payload.data_ptr = (int8_t*)&is_inplace;
	props[3].payload.max_data_len = sizeof(is_inplace);

	props[4].id = CAPI_V2_REQUIRES_DATA_BUFFERING;
	props[4].payload.data_ptr = (int8_t*)&requires_data_buffering;
	props[4].payload.max_data_len = sizeof(requires_data_buffering);

	props[5].id = CAPI_V2_NUM_NEEDED_FRAMEWORK_EXTENSIONS;
	props[5].payload.data_ptr = (int8_t*)&num_needed_framework_extensions;
	props[5].payload.max_data_len = sizeof(num_needed_framework_extensions);

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

