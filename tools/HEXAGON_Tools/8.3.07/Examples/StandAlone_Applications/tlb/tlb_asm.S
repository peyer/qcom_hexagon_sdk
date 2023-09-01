
	.text
	.p2align 2
	.p2align 4,,15
	.globl addr_in_tlb
	.type	addr_in_tlb, @function
addr_in_tlb:
	R3 = SSR					// get SSR which includes ASID
	R3 = ASL(R3,#12)			// put ASID in bits 25:20
	R0 = LSR(R0,#12)			// get VPN in low 20 bits
	R3 = INSERT(R0,#20,#0)		// 19:0 is VPN, 25:20 is ASID
	R2 = TLBP(R3)				// probe the TLB
	P0 = TSTBIT(R2,#31)			// see if it’s a hit
	if (P0) jump not_in_tlb
	R0 = EXTRACTU(R2,#8,#0)		// it was in TLB, and R0 holds the index
	jumpr r31
not_in_tlb:
	R0 = #0xff
	jumpr r31
	.size addr_in_tlb, .-addr_in_tlb

	.p2align 4,,15
	.globl get_tlb_entry
	.type	get_tlb_entry, @function
get_tlb_entry:
	r1:0 = tlbr(r0)
	jumpr r31
	.size get_tlb_entry, .-get_tlb_entry
