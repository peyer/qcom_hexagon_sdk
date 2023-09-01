/***************************************************************************
* Copyright (c) Date: Tue Aug 26 16:58:15 CDT 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:58:15 CDT 2008
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hexagon_standalone.h>
#include "l2vic_registers.h"

#define MAX 100
#define PPQ 100   //pixels per quadrant
#define QRANGE 2.0F  // quadrant range

#if __HEXAGON_ARCH__ >= 4
#define COMPUTE_THREADS 2
#else
#define COMPUTE_THREADS 5 /* __HEXAGON_ARCH__ <= 3 */
#endif

#define STACK_SIZE 16384

/*
	Functions that will be useful for the L2Vic Test Cosim
*/

#define DBG 0
#define DBG_PRINTF(x) if(DBG) printf(x);
#define DBG_PRINTF2(x,y) if(DBG) printf(x,y);

#define REL 1
#define REL_PRINTF(x) if(REL) printf(x);
#define REL_PRINTF2(x,y) if(REL) printf(x,y);

static int count = 0;

#define asm_wait()		\
    __asm__ __volatile__ (	\
	"    wait(r0)\n"	\
	: : : "r0"		\
    )

#define asm_resume()		\
    __asm__ __volatile__ (	\
    "    r0 = #7\n"		\
    "    resume(r0)\n"		\
    : : : "r0"			\
    );

void Update_L2vic(uint32 irq)
{
    DBG_PRINTF2("Entering Update_L2vic; irq = %d\n\n", irq);
    uint32 irq_bit = 1 << (irq % 32);

    // Enable the INT so they can be processed again
    *L2VIC_INT_ENABLE_SET(irq)  =  irq_bit;
}

void Init_L2vic()
{
    DBG_PRINTF("Entering Init_L2vic\n");

    uint32 irq_bit  = (1 << (IRQ1  % 32));
    uint32 irq2_bit = (1 << (IRQ2 % 32));

    if ((IRQ1 / 32) == (IRQ2 / 32))
    {
		irq_bit |= irq2_bit;
		*L2VIC_INT_ENABLE_CLEAR(IRQ1) = irq_bit;
		*L2VIC_INT_TYPE(IRQ1)        |= irq_bit;
		*L2VIC_INT_ENABLE_SET(IRQ1)   = irq_bit;
    }
    else
    {
		*L2VIC_INT_ENABLE_CLEAR(IRQ1)  = irq_bit;
		*L2VIC_INT_TYPE(IRQ1)         |= irq_bit;
		*L2VIC_INT_ENABLE_SET(IRQ1)    = irq_bit;

		*L2VIC_INT_ENABLE_CLEAR(IRQ2) = irq2_bit;
		*L2VIC_INT_TYPE(IRQ2)        |= irq2_bit;
		*L2VIC_INT_ENABLE_SET(IRQ2)   = irq2_bit;
    }

    DBG_PRINTF("\nExiting Init_L2vic\n");
}

void intr_handler (int irq)
{
    DBG_PRINTF("Entering INT_handler\n");
    uint32 vid;

    __asm__ __volatile__ (
	"%0 = VID"
	: "=r" (vid)
    );

    // Here vid defines the particular INT that is being handled
    if (vid)
    {
        count++;
        REL_PRINTF2("INT_handler: count = %d;\t", count);
        REL_PRINTF2("vid = %d\t", vid);
        REL_PRINTF("JOB DONE!!!\n");
        Update_L2vic(vid);
    }
    else
    {
        REL_PRINTF2("Other IRQ %d\n", vid);
    }

    if (count > 10)
    {
	REL_PRINTF2("\nFailed! count=%d\n\n", count);
	exit(1);
    }

#ifdef RESUME_ALL_THREADS
    asm_resume();
#endif
}

void Enable_core_interrupt()
{
	DBG_PRINTF("Entering Enable_core_interrupt\n");
	int irq = 2;
#if INTERRUPT == 31
	irq = 31;
	__asm__ __volatile__ (
	   "     r0 = #0\n"
	   "     r0 = setbit(r0,%0)\n"
	   "     r1 = iahl\n"
	   "     r1 = or(r1,r0)\n"
	   "     iahl = r1\n"
	   "     r1 = iel\n"
	   "     r1 = or(r1,r0)\n"
	   "     iel = r1\n"
	   : 
	   : "r" (irq)
	   : "r0", "r1"
	   );
#endif
	register_interrupt (irq, intr_handler);
}

/*
	End of L2Vic Test Cosim functions
*/

typedef enum {
    black = 10, violet = 9, indigo = 8, blue = 7, green = 6,
    yellow = 5, orange = 4, red = 3, white = 1
} color_t;

static const char pattern [] = {' ','\'','^','~','!','-','%','$','#','@','*','*'};

typedef struct {
    float x;
    float y;
} coord_t;

static float absolute (coord_t a);
static void compute_fractal (int * quadrant);

static color_t image_buf [4][PPQ][PPQ];
static int Mx;
static unsigned int iterations;
static char stack [COMPUTE_THREADS][STACK_SIZE];
static int quadrants [4] = {3, 2, 4, 1};

int main (int argc, char **argv)
{
    int i, j, k;

    // This is needed so the L2Vic registers are treated as device type memory
    add_translation((void *)L2VIC_BASE, (void *)L2VIC_BASE, 4);

/*
	Start of L2Vic Test Cosim functions
*/
    DBG_PRINTF ("\nEntering main\n\n");

	// Initializing INT
    Enable_core_interrupt();
    Init_L2vic();

#ifdef JUST_WAIT
    while (count < 3)
    {
        REL_PRINTF2 ("\n\nMain Code: Put thread in wait state (count=%d)\n zzzZZZ...  zzzzZZZZ\n", count);
        asm_wait();
    }

    REL_PRINTF ("\nEnd of main\n\n");
    return 0;
#endif

/*
	End of L2Vic Test Cosim functions
*/

    iterations = (argc >1)? strtol (argv [1], NULL, 10): MAX;

    // Create threads to compute four quadrants of the fractal
    // each thread is allocated a stack base and parameter

    j = 0;
    while (j < 4)
    {
        for (i = 0; i < COMPUTE_THREADS && j < 4; i++, j++)
            thread_create ((void *) compute_fractal, &stack [i][STACK_SIZE], i + 1, quadrants + j);

        thread_join (((1 << COMPUTE_THREADS) - 1) << 1);
    }

    // prints the image buffer

    for (k = 0; k < 4; k++)
    {
        for (j = 0; j < PPQ; j++)
        {
            for (i = 0; i < PPQ; i++)
                putchar (pattern [image_buf [quadrants [k] - 1][i][j]]);
            putchar ('\n');
        }
    }
#ifdef DEBUG
    printf("\nThreads done.");
#endif

    return (0);
}

static void compute_fractal (int * quadrant)
{
    int i, j, q = *quadrant - 1;
    coord_t quadrant_corner [4] = {{0, 0}, {-QRANGE, 0}, {-QRANGE, -QRANGE}, {0, -QRANGE}};
    coord_t corner = quadrant_corner[q], start;

#ifdef DEBUG
    while (1)
    {
        if (trylockMutex (&Mx))
        {
            printf("\nComputing fractal for point (%f, %f)\nwith %d \
            iterations.\n", corner.x, corner.y, iterations);
            break;
        }
    }
    unlockMutex (&Mx);
#endif
    for (j = 0; j < PPQ; j++)
    {
        for (i = 0; i < PPQ; i++)
        {
            int count = 0;
            coord_t z;
            float xtemp;

            z.x = z.y = 0.0F;
            start.x = corner.x + (i * (QRANGE / PPQ));
            start.y = corner.y + (j * (QRANGE / PPQ));
            while ((count < iterations) && (absolute (z) <= 4.0F))
            {
                xtemp  = (z.x * z.x) - (z.y * z.y) + start.x;
                z.y    = 2.0F * (z.x * z.y) + start.y;
                z.x    = xtemp;
                count++;
            }

            // Fractal coloring
            if (absolute (z) <= 4.0F)
                image_buf [q][i][j] = 10;
            else if (count > (int)(MAX * 0.9F))
                image_buf [q][i][j] = 9;
            else if (count > (int)(MAX * 0.8F))
                image_buf [q][i][j] = 8;
            else if (count > (int)(MAX * 0.6F))
                image_buf [q][i][j] = 7;
            else if (count > (int)(MAX * 0.4F))
                image_buf [q][i][j] = 6;
            else if (count > (int)(MAX * 0.3F))
                image_buf [q][i][j] = 5;
            else if (count > (int)(MAX * 0.1F))
                image_buf [q][i][j] = 4;
            else if (count > (int)(MAX * 0.05F))
                image_buf [q][i][j] = 3;
            else if (count > (int)(MAX * 0.01F))
                image_buf [q][i][j] = 2;
            else if (count > (int)(MAX * 0.001F))
                image_buf [q][i][j] = 1;
            else image_buf [q][i][j] = 0;
        }
    }
#ifdef DEBUG
    lockMutex (&Mx);
    {
        printf("\nDone computing fractal for thread %d.", thread_get_tnum());
    }
    unlockMutex (&Mx);
#endif
}

static float absolute (coord_t a)
{
    return ((a.x * a.x) + (a.y * a.y));
}
