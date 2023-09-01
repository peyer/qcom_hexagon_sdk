/*==============================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

#include "HAP_farf.h"
#include "calculator.h"
#include "calculator_dsp.h"
#include "qurt.h"
#include "verify.h"

#define GLOBAL_VAL 100
#define STACK_SIZE              4 * 1024
static int global_val = GLOBAL_VAL;

int calculator_plus_sum(const int *input, int count, int64 *res)
{
    Calculator calculator(input, count);
    *res = calculator.sum();
    FARF(ALWAYS, "Calculator sum returned %lld", *res);
    FARF(ALWAYS, "Global value =  %d", ++global_val);
    return 0;
}

int calculator_plus_static_sum(const int *input, int count, int64 *res1, int64 *res2){
    static Calculator calulator_1 = Calculator(input, count);
    static Calculator* calulator_2 = new Calculator(input, count);
    FARF(ALWAYS, "Global value =  %d", ++global_val);

    *res1 = calulator_1.sum();
    *res2 = calulator_2->sum();

    FARF(ALWAYS, "Calculator static sum returned %lld %lld", *res1, *res2);
    return 0;
}

typedef struct {
    const char* filename;
    int64 res;
}iostream_worker_data;

static void iostream_worker(void *arg){
    int nErr = 0;
    iostream_worker_data* data = (iostream_worker_data*)arg;
    Calculator calculator(data->filename);
    std::ofstream stream1("calculator.output");

    VERIFY(data != NULL);
    VERIFY(data->filename != NULL);

    data->res = calculator.sum();
    FARF(ALWAYS, "Calculator iostream sum returned %lld", data->res);
    FARF(ALWAYS, "Global value =  %d", ++global_val);

    FARF(ALWAYS, "Writing output to file");
    stream1 << data->res;

bail:
    if (nErr){
        FARF(ALWAYS, "Error in iostream_worker");
    }
    return;
}

int calculator_plus_iostream_sum(const char* filename, int64*res){

    int nErr = 0;
    qurt_thread_attr_t attr;
    void* threadStack = NULL;
    qurt_thread_t threadId = -1;
    int status = 0;

    iostream_worker_data data;
    VERIFY(filename!= NULL && res != NULL);
    data.filename = filename;
    qurt_thread_attr_init (&attr);
    qurt_thread_attr_set_stack_size (&attr, STACK_SIZE);
    qurt_thread_attr_set_priority (&attr, 0xC0);
    qurt_thread_attr_set_name (&attr, (char*)"iost");
    VERIFY(NULL != (threadStack = malloc(STACK_SIZE * sizeof(uint8))));
    qurt_thread_attr_set_stack_addr (&attr, (unsigned long long *)threadStack);
    VERIFY(QURT_EOK == qurt_thread_create(&threadId, &attr, iostream_worker,
                                          &data ));

    qurt_thread_join(threadId, &status);
    *res = data.res;

bail:
    if(threadStack)
        free (threadStack);

    return nErr;
}



Calculator::Calculator(const int* seq, int vecLen){
    input_vec.assign(seq, seq+vecLen);
    FARF(ALWAYS, "Global value =  %d", ++global_val);
    FARF(ALWAYS, "In Calculator ctor 1");
}

Calculator::Calculator(const char* filename){
    int num = 0;
    std::ifstream stream1(filename);

    FARF(ALWAYS, "Global value =  %d", ++global_val);
    FARF(ALWAYS, "In Calculator ctor 2");

    while (stream1 >> num){
        input_vec.push_back(num);
    }
}

int64 Calculator::sum()
{
    int64 res = 0;
    for (std::vector<int>::iterator it = input_vec.begin(); it != input_vec.end(); ++it){
        res += *it;
    }
    return res;
}
