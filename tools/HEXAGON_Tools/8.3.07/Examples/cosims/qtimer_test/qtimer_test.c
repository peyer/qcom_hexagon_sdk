/******************************************************************************
**
**	Qtimer Example
**
** This example initializes two timers that cause interrupts at different
** intervals.  The thread 0 will sit in wait mode till the interrupt is
** serviced then return to wait mode.
**
******************************************************************************/
#include "qtimer.h"
int
main()
{
	int i;
	exit_flag = 0;
    printf ("\nCSR base=0x%x; L2VIC base=0x%x\n", CSR_BASE, L2VIC_BASE);
    printf ("QTimer1 will go off 20 times (once every 1/%d sec).\n",
					(QTMR_FREQ)/(ticks_per_qtimer1));
	printf ("QTimer2 will go off 2 times (once every 1/%d sec).\n\n",
					(QTMR_FREQ)/(ticks_per_qtimer2));

	add_translation ((void *)CSR_BASE, (void *)CSR_BASE, 4);

    enable_core_interrupt();

    init_l2vic();
	// initialize qtimers 1 and 2
    init_qtimers(3);

    while (qtimer2_cnt < 2)
    {
		// Thread 0 waits for interrupts
		asm_wait();
    }
    return 0;
}
