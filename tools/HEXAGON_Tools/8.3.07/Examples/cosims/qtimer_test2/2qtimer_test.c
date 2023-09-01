/******************************************************************************
**
**	Qtimer Example
**
** This example initializes two timers that cause interrupts at different
** intervals.  Each thread will have a different IRQ handler and IRQ source.
** thread 0 = IRQ2, thread 1 = IRQ3, thread 2 = IRQ4, thread 3 = IRQ5
**
******************************************************************************/
#include "2qtimer.h"

char __attribute__ ((aligned (16))) stack1[STACK_SIZE];
char __attribute__ ((aligned (16))) stack2[STACK_SIZE];
//char __attribute__ ((aligned (16))) stack3[STACK_SIZE];

int
main()
{
	int i;
	exit_flag = 0;
    printf ("\nCSR_base1=0x%x, CSR_base2=0x%x, L2VIC base=0x%x\n", CSR_BASE1, CSR_BASE2, L2VIC_BASE);
    printf ("QTimer1 will cause interrupt 12 times (once every 1/%d sec).\n",
					(QTMR_FREQ)/(ticks_per_qtimer1));
	printf ("QTimer2 will cause interrupt 2 times for normal and 2 for fast (once every 1/%d sec).\n\n",
					(QTMR_FREQ)/(ticks_per_qtimer2));

	add_translation ((void *)CSR_BASE1, (void *)CSR_BASE1, 4);

	thread_create((void *)thread1, &stack1[STACK_SIZE-16], 1, NULL);
	thread_create((void *)thread2, &stack2[STACK_SIZE-16], 2, NULL);
	//thread_create((void *)thread3, &stack3[STACK_SIZE-16], 3, NULL);

	// test irqs normally
	printf("Testing normal l2vic irqs\n");
	if(test_irqs(NORMAL))
	{
		printf("failed normal irq tests\n");
		exit(-1);
	}
	// test irqs with fast interrupt enabled for irq[1]
	printf("\nTesting fast l2vic irqs\n");
	// turn off qtimers
	init_qtimers(0xf, OFF);
	fast_irq_enable[1] = ON;
	if(test_irqs(FAST))
	{
		printf("failed fast irq tests\n");
		exit(-1);
	}


    return 0;
}
