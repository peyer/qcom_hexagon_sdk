
#include <stdio.h>
#include <hexagon_standalone.h>

#if __HEXAGON_ARCH__ <= 4
#define NUM_TLB_ENTRIES 63
#else
#define NUM_TLB_ENTRIES 127
#endif

unsigned long long tlb_entries[NUM_TLB_ENTRIES];

//unsigned long long get_tlb_entry(int num);

#define VIRT_PAGE(hi)  (hi & 0xfffff)
#define PAGE_VALS(lo)  (lo == 1 ? "4KB" : (lo == 2 ? "16KB" : (lo == 4 ? "64KB" : (lo == 8 ? "256KB" : (lo == 16 ? "1MB" : (lo == 32 ? "4MB" : "16MB"))))))
#define ASID(hi)       ((hi >> 20) & 0x7f)
#define P_READ(lo)	   ((lo >> 29) & 0x01)
#define P_WRITE(lo)	   ((lo >> 30) & 0x01)
#define P_EXEC(lo)	   ((lo >> 31) & 0x01)
#define P_USER(lo)	   ((lo >> 28) & 0x01)
#define P_CCCC(lo)	   ((lo >> 24) & 0x0f)
#define P_S(lo)	       ((lo >> 0) & 0x01)
#define P_V(hi)	       ((hi >> 31) & 0x01)
#define P_G(hi)	       ((hi >> 30) & 0x01)
#define P_EP(hi)	   ((hi >> 29) & 0x01)
#define P_A1(hi)	   ((hi >> 28) & 0x01)
#define P_A0(hi)	   ((hi >> 27) & 0x01)

#define PAGE_SIZE(lo, mask)  PAGE_VALS((lo & mask))

static unsigned long long get_tlb_entry(int tlb_num)
{
	unsigned long long tlb_addr;

	asm volatile (
		"%0 = tlbr(%1)\n"
		: "=r"(tlb_addr)
		: "r"(tlb_num));
	return(tlb_addr);
}
unsigned int get_phys_mask(unsigned int tlblo)
{
	int i = 0;
	unsigned int mask = 0;

	for(i = 0; i <= 7; i++)
	{
		mask |= (0x01 << i);

		// first set bit defines mask and page size
		if(tlblo & (0x01 << i))
		{
			return mask;
		}
	}

	return 0;
}

unsigned int get_phys_page(unsigned int lo, unsigned int hi, unsigned int mask)
{
	// get bits [34:12] defined at [23:1]
	unsigned int pp = ((lo >> 1) & 0x7fffff);
	// add bit [35]
	if(hi & (0x01 << 29))
		pp |= 0x800000;

	// mask page size bits
	pp &= ~(mask >> 1);

	return pp;
}

void print_tlbs(void)
{
	int tlbnum = 0;

	printf("\n");

	printf("\nIdx:  VPAGE    PPAGE       SIZE   ASID  R W X U  CCCC  S  V  G  EP  A1 A0  Raw_hi     Raw_lo\n");

	// collect all the tlb's
	for(tlbnum = 0; tlbnum <= NUM_TLB_ENTRIES; tlbnum++)
	{
		tlb_entries[tlbnum] = get_tlb_entry(tlbnum);
		unsigned int tlbhi = (tlb_entries[tlbnum]) >> 32;
		unsigned int tlblo = (tlb_entries[tlbnum]) & 0xffffffff;

		// only print non zero entries
		if(tlb_entries[tlbnum] != 0x00)
		{
			//printf("Tlb entry %d = 0x%016llx\n", tlbnum, tlb_entries[tlbnum] >> 1);
			unsigned int mask = get_phys_mask(tlblo);
			unsigned int PPN = get_phys_page(tlblo, tlbhi, mask);
			printf("%4d  0x%05x  0x%06x    %-5s  0x%02x  %d %d %d %d  0x%0x   %d  %d  %d  %d   %d  %d  0x%x 0x%x\n", tlbnum, VIRT_PAGE(tlbhi), PPN, PAGE_SIZE(tlblo, mask), ASID(tlbhi),
					P_READ(tlblo), P_WRITE(tlblo), P_EXEC(tlblo), P_USER(tlblo), P_CCCC(tlblo), P_S(tlblo), P_V(tlbhi), P_G(tlbhi), P_EP(tlbhi), P_A1(tlbhi), P_A0(tlbhi), tlbhi, tlblo);
		}
	}
}

