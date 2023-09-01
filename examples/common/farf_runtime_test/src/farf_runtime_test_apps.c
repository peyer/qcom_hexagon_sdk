#include <stdio.h>
#include <string.h>

#include "farf_runtime_test.h"

#define HAP_LEVEL_RUNTIME   (1 << 5)
#define BAD_FILENAME        "vivek"
#define GOOD_FILENAME       "farf_runtime_test_adsp.c"
#define PROMPT              printf("Press any key+Enter to continue, q+Enter to quit");\
                            if (getchar() == 'q')\
                              break;\
                              getchar();
#define TEST_MULTI_CNT      20

void testMasks(){
    // Iterate over all possible values of the mask
    for (int i = 0; i < HAP_LEVEL_RUNTIME; i++){
        // Set the mask
        farf_runtime_test_setMask(i);
        farf_runtime_test_run();
    }
}

void testBadFileNotLogged(){
    // Set the filename to a random filename and ensure that no output is produced
    farf_runtime_test_setMaskandFilename(0x1f,BAD_FILENAME);
    farf_runtime_test_run();
}

void testGoodFileLogged(){
    // Set the filename to the correct file and ensure that all messages are printed
    farf_runtime_test_setMaskandFilename(0x1f,GOOD_FILENAME);
    farf_runtime_test_run();
}

int main(){

      do{

        printf("\n\nopen a cmd shell and start logcat: >adb shell logcat\n");
        printf("logcat not enabled for basic farf message test:\n");
        printf(" verify no logcat messages appear\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        PROMPT;
        farf_runtime_test_run();
       
        printf("\n\nlogcat not enabled for log mask test:\n");
        printf(" verify no logcat messages appear\n");
        printf(" verify all 32 mask combinations for the five runtime configured log messages in QXDM traces\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces appear along with each mask runtime mask test\n");
        PROMPT;
        testMasks();      

        printf("\n\nenabling logs for a file using a bad filename:\n");
        printf(" verify no logcat messages appear\n");
        printf(" verify no runtime messages appear in QXDM traces\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        PROMPT;
        testBadFileNotLogged();

        printf("\n\nenabling logs for a file using a good filename:\n");
        printf(" verify no logcat messages appear\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        printf(" verify the five runtime messages appear in QXDM traces\n");
        PROMPT;
        testGoodFileLogged();

        printf("\n\nrunning multiple ALWAYS log traces:\n");
        printf(" verify no logcat messages appear\n");
        printf(" verify %d messages appear in QXDM traces\n", TEST_MULTI_CNT);
        PROMPT;
        farf_runtime_test_run_multiple(TEST_MULTI_CNT);

        printf("\n\nenabling logcat:\n");
        printf(" verify logcat messages appear in logcat shell\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        printf(" verify the five runtime messages appear in QXDM traces\n");
		printf(" Create a file called 'farf_runtime_test.farf' in \\vendor\\lib\\rfsa\\adsp in the device and write '0x1f' in it.\n");
        PROMPT;
        farf_runtime_test_run();

        printf("\n\ndisabling logcat:\n");
        printf(" verify no logcat messages appear in logcat shell\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        printf(" verify the five runtime messages appear in QXDM traces\n");
		printf(" Remove the 'farf_runtime_test.farf' from \\vendor\\lib\\rfsa\\adsp in the device\n");
        PROMPT;
        farf_runtime_test_run();

        printf("\n\nenabling logcat again after being disabled:\n");
        printf(" verify logcat messages appear in logcat shell\n");
        printf(" verify Low, Medium, High, Error, and Fatal messages appear in QXDM traces\n");
        printf(" verify the five runtime messages appear in QXDM traces\n");
        printf(" Create the file 'farf_runtime_test.farf' again in \\vendor\\lib\\rfsa\\adsp in the device and write '0x1f' in it.\n");
		PROMPT;
        farf_runtime_test_run();

        printf("\n\nend of test\n");
        
        break;
    }while(1);

    return 0;
}
