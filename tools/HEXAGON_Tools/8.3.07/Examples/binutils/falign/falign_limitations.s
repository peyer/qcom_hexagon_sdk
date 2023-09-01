	loop0 (foo, #10)

	.align	8

	.falign

1:
	{ r0 = r1
	  r2 = r3
	  r4 = r5 }

	.falign

foo:
	{ r12 = r13
	  r14 = r15
	  r16 = r17
	  r18 = r19 }:endloop0

	{ r20 = r21
	  r22 = r23 }

2:
	{ r24 = r25
	  r26 = r27
	  r28 = r29
	  r30 = r31 }

	.falign

3:
	{ r8 = r9
	  r9 = r10
	  r11 = r12
	  r13 = r14 }
