#include <stdio.h>
#include <string.h>

// Enable all FARF levels
#define FARF_LOW    1
#define FARF_MEDIUM 1
#define FARF_HIGH   1
#define FARF_ERROR  1
#define FARF_FATAL  1

#include "HAP_farf.h"
#include "farf_runtime_test.h"


uint32 farf_runtime_test_run(){
    FARF(LOW,"Low FARF message");
    FARF(MEDIUM,"Medium FARF message");
    FARF(HIGH,"High FARF message");
    FARF(ERROR,"Error FARF message");
    FARF(FATAL,"Fatal FARF message");

    FARF(RUNTIME_LOW,"Runtime Low FARF message");
    FARF(RUNTIME_MEDIUM,"Runtime Medium FARF message");
    FARF(RUNTIME_HIGH,"Runtime High FARF message");
    FARF(RUNTIME_ERROR,"Runtime Error FARF message");
    FARF(RUNTIME_FATAL,"Runtime Fatal FARF message");

    return 0;
}
uint32 farf_runtime_test_run_multiple(uint32 num_iter){
    int i = 0;
    for (i = 0; i < num_iter; ++i){
        FARF(ALWAYS, "FARF iteration %d", i);
    }
    return 0;
}

uint32 farf_runtime_test_setMask(uint32 mask){
    HAP_setFARFRuntimeLoggingParams(mask, NULL, 0);
    return 0;
}

uint32 farf_runtime_test_setMaskandFilename(uint32 mask, const char* filename){
    const char* files[] = {filename};
    HAP_setFARFRuntimeLoggingParams(mask, files, 1);
    return 0;
}
