/***************************************************************************
* Copyright (c) 2008-2016 Qualcomm Technologies, Inc.  All Rights Reserved
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hexagon_standalone.h>

#define MAX 100
#define PPQ 100   //pixels per quadrant
#define QRANGE 2.0F  // quadrant range

#define MIN_HW_THREADS 2
#define MAX_HW_THREADS 4
#define STACK_SIZE 16384

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
static int get_hw_thread_count(void);

static color_t image_buf [4][PPQ][PPQ];
static int Mx;
static unsigned int iterations;
static char stack [MAX_HW_THREADS-1][STACK_SIZE];
static int quadrants [4] = {3, 2, 4, 1};

int main (int argc, char **argv)
{
    int i, j, k;
    int hw_threads = get_hw_thread_count();

#ifdef DEBUG
    printf ("Target has %d hw threads\n", hw_threads);
#endif

    iterations = (argc >1)? strtol (argv [1], NULL, 10): MAX;

    // Create threads to compute four quadrants of the fractal
    // each thread is allocated a stack base and parameter

    j = 0;
    while (j < 4)
    {
        for (i = 1; i < hw_threads && j < 4; i++, j++)
        {
            thread_create ((void *) compute_fractal, &stack [i-1][STACK_SIZE], i, quadrants + j);
        }

        if (j < 4)
        {
            compute_fractal (quadrants + j);
            j++;
        }

        thread_join (((1 << hw_threads) - 1) & ~1);
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
            printf("\nHw thread %d is computing fractal for point (%f, %f)\nwith %d \
            iterations.\n", thread_get_tnum(), corner.x, corner.y, iterations);
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

#if OPTIMIZE_STALL >= 1
            // The optimized version helps alleviate interlock
            // stalls between producer/consumer packets in the
            // original version for a perf improvement of 10%
            float a = 0.0F;
            float b = 0.0F;
            float c = 0.0F;
            while ((count < iterations) && ((a + b) <= 4.0F))
            {
                z.x = a - b + start.x;
                z.y = 2.0F * c + start.y;
                a   = z.x * z.x;
                b   = z.y * z.y;
                c   = z.x * z.y;
                count++;
            }
#else
            while ((count < iterations) && (absolute (z) <= 4.0F))
            {
                xtemp  = (z.x * z.x) - (z.y * z.y) + start.x;
                z.y    = 2.0F * (z.x * z.y) + start.y;
                z.x    = xtemp;
                count++;
            }
#endif

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


static void RollCall(int *here)
{
    *here = thread_get_tnum();
}


static int get_hw_thread_count()
{
    int roster[MAX_HW_THREADS];
    unsigned int modectl = 0;
    unsigned int joinmask = 0;
    int i;

    __asm__ __volatile__ (
        "%0 = MODECTL"
        : "=r" (modectl)
    );

#ifdef DEBUG
    printf ("modectl = 0x%x\n", modectl);
#endif

    for (i = MIN_HW_THREADS; i < MAX_HW_THREADS; i++)
    {
        if (modectl & (1 << i))
        {
            // Hw Thread 'i' is already running
            roster[i] = i;

#ifdef DEBUG
            printf ("Hw Thread %d already running\n", i);
#endif
        }
        else
        {
            joinmask |= (1 << i);
            roster[i] = 0;
            thread_create ((void *) RollCall, &stack[i-1][STACK_SIZE], i, &roster[i]);
        }
    }

    thread_join (joinmask);

    for (i = MIN_HW_THREADS; i < MAX_HW_THREADS; i++)
    {
        if (roster[i] != i) break;
    }

    return i;
}
