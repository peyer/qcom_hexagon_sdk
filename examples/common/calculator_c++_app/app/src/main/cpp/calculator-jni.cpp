#include <jni.h>
#include <string>
#include <android/log.h>
#include <stdlib.h>
#include <rpcmem.h>
#include "remote.h"
#include <calculator.h>

using namespace std;
class calculator {

public:
    int64 result;
    int nErr;
}; // object;

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_qualcomm_calculator_MainCalculator_init(JNIEnv *env, jobject instance,
                                                         jstring skel_location_) {
    const char *skel_location = env->GetStringUTFChars(skel_location_, 0);
    setenv("ADSP_LIBRARY_PATH", skel_location, 1);
    env->ReleaseStringUTFChars(skel_location_, skel_location);
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_qualcomm_calculator_MainCalculator_sum(JNIEnv *env, jobject instance,
                                                        jintArray vec_, jint len) {
    jint *vec = env->GetIntArrayElements(vec_, NULL);
    calculator *object = new calculator();
    object->result = 0;
    object->nErr = 0;
    int *test = 0;
    int alloc_len = sizeof(*test) * len;
    remote_handle64 handle= -1;
    char calculator_URI_Domain[128];

    rpcmem_init();
    __android_log_print(ANDROID_LOG_DEBUG, "CALCULATOR", "ENTERING");

    if (0 == (test = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, alloc_len))) {
        object->nErr = 1;
        __android_log_print(ANDROID_LOG_DEBUG, "CALCULATOR", "rpcmem_alloc failed with nErr = %d",
                            object->nErr);
        goto bail;
    }
    for (int i=0; i<len; i++){
        test[i] = vec[i];
    }

    strlcpy(calculator_URI_Domain, calculator_URI, strlen(calculator_URI) + 1);
    // first try opening handle on CDSP.
    strlcat(calculator_URI_Domain, "&_dom=cdsp", strlen(calculator_URI_Domain)+strlen("&_dom=cdsp") + 1);
    object->nErr = calculator_open(calculator_URI_Domain, &handle);

    // if CDSP is not present, try ADSP.
    if (object->nErr != 0)
    {
        printf("cDSP not detected on this target (error code %d), attempting to use aDSP\n", object->nErr);
        strlcat(calculator_URI_Domain, "&_dom=adsp", strlen(calculator_URI_Domain)+strlen("&_dom=adsp") + 1);
        object->nErr = calculator_open(calculator_URI_Domain, &handle);
    }
    if (object->nErr != 0)
    {
        printf("Failed to open domain");
        goto bail;
    }

    object->nErr = calculator_sum(handle, test, len, &object->result);
    if (object->nErr) {
        printf("ERROR: Failed to compute calculator_sum\n");
    }
    __android_log_print(ANDROID_LOG_DEBUG, "CALCULATOR", "EXITING with %d", object->nErr);

    bail:
    if (test) { rpcmem_free(test); }
    if (object) { delete object; }
    if (handle) { calculator_close(handle); }
    rpcmem_deinit();
    env->ReleaseIntArrayElements(vec_, vec, 0);
    return object->result;
}