/*****************************************************************
# Copyright (c) 2013 QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef HEXAGON_SIM_TIMER_H
#define HEXAGON_SIM_TIMER_H

/** Disable Simulator Timer Option. */
//#define HEXAGON_SIM_TIMER_DISABLE 1

#include <stdio.h>

/*
   Use these functions to time your code, like this:

main()
{
  hexagon_sim_init_timer();
  ...
  hexagon_sim_start_timer();
  CODE_I_CARE_ABOUT();
  hexagon_sim_end_timer();
  ...
  hexagon_sim_show_timer(stdout);
}

*/

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize and start timer. */
void hexagon_sim_init_timer(void);
/** Clear timer to start values. */
void hexagon_sim_clear_timer(void);
/** Start timer. */
void hexagon_sim_start_timer(void);
/** End/Stop timer. */
void hexagon_sim_end_timer(void);
/** Show/Print timer values to file. */
void hexagon_sim_show_timer(FILE *file);

#ifdef HEXAGON_SIM_TIMER_DISABLE
/** STUB: return current thread tcycles DEPRECATED on V5+ Architectures. */
#define hexagon_sim_read_cycles() 0

/** STUB: return thread tnum's cycle DEPRECATED on V5+ Architectures. */
#define hexagon_sim_read_tcycles(tnum) 0

/** STUB: return processor cycles. */
#define hexagon_sim_read_pcycles() 0
#else
/** return current thread tcycles DEPRECATED on V5+ Architectures. */
unsigned long long hexagon_sim_read_cycles(void);

/** return thread tnum's cycles DEPRECATED on V5+ Architectures.
    tnum [1..number of threads] (0 is invalid thread number). */
unsigned long long hexagon_sim_read_tcycles(int tnum);

/** return processor cycles. */
unsigned long long hexagon_sim_read_pcycles(void);
#endif

/** Simulator profiling on. */
void hexagon_sim_prof_on(void);
/** Simulator profiling off. */
void hexagon_sim_prof_off(void);
/** Write event to simulator log file. */
int  hexagon_sim_log(char *event);

/***
/*** backward compatibility aliases
 ***/
void q6sim_init_timer(void);
/** Clear timer to start values. */
void q6sim_clear_timer(void);
/** Start timer. */
void q6sim_start_timer(void);
/** End/Stop timer. */
void q6sim_end_timer(void);
/** Show/Print timer values to file. */
void q6sim_show_timer(FILE *file);

#ifdef Q6SIM_TIMER_DISABLE
/** STUB: return current thread tcycles DEPRECATED on V5+ Architectures. */
#define q6sim_read_cycles() 0

/** STUB: return thread tnum's cycle DEPRECATED on V5+ Architectures. */
#define q6sim_read_tcycles(tnum) 0

/** STUB: return processor cycles. */
#define q6sim_read_pcycles() 0
#else
/** return current thread tcycles DEPRECATED on V5+ Architectures. */
unsigned long long q6sim_read_cycles(void);

/** return thread tnum's cycles DEPRECATED on V5+ Architectures.
    tnum [1..number of threads] (0 is invalid thread number). */
unsigned long long q6sim_read_tcycles(int tnum);

/** return processor cycles. */
unsigned long long q6sim_read_pcycles(void);
#endif

/** Simulator profiling on. */
void q6sim_prof_on(void);
/** Simulator profiling off. */
void q6sim_prof_off(void);
/** Write event to simulator log file. */
int  q6sim_log(char *event);

#ifdef __cplusplus
};
#endif

#endif
