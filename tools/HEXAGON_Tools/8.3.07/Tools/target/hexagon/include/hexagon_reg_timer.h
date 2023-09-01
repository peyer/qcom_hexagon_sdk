/*****************************************************************
# Copyright (c) 2013 QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef HEXAGON_REG_TIMER_H
#define HEXAGON_REG_TIMER_H

/** Disable Register Timer Option. */
//#define HEXAGON_REG_TIMER_DISABLE 1

#include <stdio.h>

/*
   Use these functions to time your code, like this:

main()
{
  hexagon_reg_init_timer();
  ...
  hexagon_reg_start_timer();
  CODE_I_CARE_ABOUT();
  hexagon_reg_end_timer();
  ...
  hexagon_reg_show_timer(stdout);
}

*/

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize and start timer. */
void hexagon_reg_init_timer(void);
/** Clear timer to start values. */
void hexagon_reg_clear_timer(void);
/** Start timer. */
void hexagon_reg_start_timer(void);
/** End/Stop timer. */
void hexagon_reg_end_timer(void);
/** Show/Print timer values to file. */
void hexagon_reg_show_timer(FILE *file);

#ifdef HEXAGON_REG_TIMER_DISABLE
/** STUB: return processor cycles. */
#define hexagon_reg_read_pcycles() 0
#else
/** return processor cycles. */
unsigned long long hexagon_reg_read_pcycles(void);
#endif

/** Porting STUB: Profiling on marker. */
void hexagon_reg_prof_on(void);
/** Porting STUB: Profiling off marker. */
void hexagon_reg_prof_off(void);

/** return revision register. */
unsigned int hexagon_reg_read_rev(void);

/** return syscfg register. */
unsigned int hexagon_reg_read_syscfg(void);

/*** backward compatibility aliases  ***/
/** Initialize and start timer. */
void q6reg_init_timer(void);
/** Clear timer to start values. */
void q6reg_clear_timer(void);
/** Start timer. */
void q6reg_start_timer(void);
/** End/Stop timer. */
void q6reg_end_timer(void);
/** Show/Print timer values to file. */
void q6reg_show_timer(FILE *file);

#ifdef Q6REG_TIMER_DISABLE
/** STUB: return processor cycles. */
#define q6reg_read_pcycles() 0
#else
/** return processor cycles. */
unsigned long long q6reg_read_pcycles(void);
#endif

/** Porting STUB: Profiling on marker. */
void q6reg_prof_on(void);
/** Porting STUB: Profiling off marker. */
void q6reg_prof_off(void);

/** return revision register. */
unsigned int q6reg_read_rev(void);

/** return syscfg register. */
unsigned int q6reg_read_syscfg(void);

#ifdef __cplusplus
};
#endif

#endif



