#define   TEST_DEC      0              // 0 -bit-exactness test

//
// SIMULATION SETUP
//
#define MAX_NUM_FRAME_ERR   (160)      // NUMBER OF FRAME ERRORS TO STOP SIMULATION
#define MAX_FRAME_NUM       (1000000)  // MAXIMUM NUMBER of FRAME TO RUN
#define NUM_ITER            8          // NUMBER of DECODING ITERATIONS
//
// DECODER PARAMTERS
//
#define INTG_BIT     4
#define FRAC_BIT     3

//
// SYSTEM SETUP
//
#define K (4)                          // CONSTRAINT LENGTH OF ENCODER
#define CODE_RATE (1.0/3.0)            // COMPOSITE CODE RATE
#define FRAME_SIZE (640)               // NUMBER OF INFO BITS IN A FRAME


