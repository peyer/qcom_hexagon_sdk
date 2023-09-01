	.text
	.p2align 5
	.global hvx_copy_line
hvx_copy_line:
	/* hvx_copy_line(tx,rx,length) */
	r2 = lsr(r2,#6)
	loop0(1f,r2)
	.falign
1:
	{
		v0.cur = vmem(r1++#1)
		vmem(r0++#1) = v0.new
	}:endloop0
	jumpr r31

