/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* This header file must be included to use the simulator cycle
   timers */
#include <hexagon_sim_timer.h>


/* This function represents an important piece of code for which the
   programmer would like to measure performance */
void CODE_I_CARE_ABOUT();

int main()
{
  /* The timers must always be initialized before use */
  hexagon_sim_init_timer();

  /* This block of code shows how to use the*/
  {
    /* This starts the timer, internally recording the starting time */
    hexagon_sim_start_timer();
    /* This region of code can be anything that the programmer would
       like to measure; in this case, it is simply a function call */
    CODE_I_CARE_ABOUT();
    /* This ends the timer, internally recording the stopping time */
    hexagon_sim_end_timer();
  }

  /* This block of code shows how to directly read the cycle timer
     (for this thread).  You can use this if you want to measure
     different pieces of code at the same time.  */
  hexagon_sim_start_timer();
  {
    unsigned long long start, stop;
    start = hexagon_sim_read_cycles();
    CODE_I_CARE_ABOUT();
    stop = hexagon_sim_read_cycles();
    printf("Start = %lld\n", start);
    printf("Stop = %lld\n", stop);
    printf("Time = %lld\n", stop - start);
  }
  hexagon_sim_end_timer();

  /* This prints out the timing statistics for the hexagon_sim_start_timer
     and hexagon_sim_end_timer calls.  It will print the total/min/max/avg
     over all of the sets of timer calls.  */
  printf("hexagon_sim_show_timer():\n");
  hexagon_sim_show_timer(stdout);
  return 0;
}




/* This is just an example piece of code that we want to measure. */
void CODE_I_CARE_ABOUT() {
  int i = 0;
  for (i = 0 ; i < 1000; i++) {
    printf("%d", i % 2);
  }
  printf("\n");
}
