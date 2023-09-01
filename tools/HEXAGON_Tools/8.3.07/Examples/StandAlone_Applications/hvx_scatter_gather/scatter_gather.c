
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <hexagon_types.h>
#include <hexagon_standalone.h>
#include <hvx_hexagon_protos.h>
#include <hexagon_sim_timer.h>

// define the number of rows/cols in a square matrix
//#define MATRIX_SIZE 32
#ifdef HVX_DOUBLE
#define MATRIX_SIZE 64
#else
#define MATRIX_SIZE 32	
#endif

// define the size of the scatter buffer
#define SCATTER_BUFFER_SIZE (MATRIX_SIZE*MATRIX_SIZE)

#define SCATTER16_BUF_SIZE  2*SCATTER_BUFFER_SIZE
#define SCATTER32_BUF_SIZE  4*SCATTER_BUFFER_SIZE

#define GATHER16_BUF_SIZE  2*MATRIX_SIZE
#define GATHER32_BUF_SIZE  4*MATRIX_SIZE

const unsigned int TCM_BASE						= 0xD800 << 16;
const unsigned int VTCM_BASE_ADDRESS			= TCM_BASE + (2 << 20);
const unsigned int VTCM_SCATTER16_ADDRESS		= VTCM_BASE_ADDRESS;
const unsigned int VTCM_GATHER16_ADDRESS		= VTCM_BASE_ADDRESS + SCATTER16_BUF_SIZE;
const unsigned int VTCM_SCATTER32_ADDRESS		= VTCM_GATHER16_ADDRESS + GATHER16_BUF_SIZE;
const unsigned int VTCM_GATHER32_ADDRESS		= VTCM_SCATTER32_ADDRESS + SCATTER32_BUF_SIZE;
const unsigned int VTCM_SCATTER16_32_ADDRESS	= VTCM_GATHER32_ADDRESS + GATHER32_BUF_SIZE;
const unsigned int VTCM_GATHER16_32_ADDRESS		= VTCM_SCATTER16_32_ADDRESS + SCATTER16_BUF_SIZE;

// the vtcm base address
unsigned char  *vtcm_base	= (unsigned char  *)VTCM_BASE_ADDRESS;

// scatter gather 16 bit elements using 16 bit offsets
unsigned short *vscatter16	= (unsigned short *)VTCM_SCATTER16_ADDRESS;
unsigned short *vgather16	= (unsigned short *)VTCM_GATHER16_ADDRESS;

// scatter gather 32 bit elements using 32 bit offsets
unsigned int   *vscatter32	= (unsigned int   *)VTCM_SCATTER32_ADDRESS;
unsigned int   *vgather32	= (unsigned int   *)VTCM_GATHER32_ADDRESS;

// scatter gather 16 bit elements using 32 bit offsets
unsigned short *vscatter16_32	= (unsigned short *)VTCM_SCATTER16_32_ADDRESS;
unsigned short *vgather16_32	= (unsigned short *)VTCM_GATHER16_32_ADDRESS;


// declare the arrays of offsets
unsigned short half_offsets[MATRIX_SIZE];
unsigned int   word_offsets[MATRIX_SIZE];

// declare the arrays of values
unsigned short half_values[MATRIX_SIZE];
unsigned int   word_values[MATRIX_SIZE];


// make this big enough for all the intrinsics
unsigned int region_len	= 4*SCATTER_BUFFER_SIZE - 1;

// optionally add sync instructions
#define SYNC_VECTOR 1

// optionally print cycle counts
#define PRINT_CYCLE_COUNTS 1

#if PRINT_CYCLE_COUNTS
unsigned long long start_cycles;
#define START_CYCLES start_cycles = hexagon_sim_read_pcycles();
#define PRINT_CYCLES(x) printf(x,  hexagon_sim_read_pcycles() - start_cycles);
#else
#define START_CYCLES
#define PRINT_CYCLES(x)
#endif

// define a scratch area for debug and prefill
//#define SCRATCH_SIZE	0x2800
#ifdef HVX_DOUBLE
#define SCRATCH_SIZE	0x8800
#else
#define SCRATCH_SIZE	0x2800	
#endif

// fill vtcm scratch with ee
void prefill_vtcm_scratch(void)
{
	memset((void *)VTCM_BASE_ADDRESS, 'e', SCRATCH_SIZE * sizeof(char));
}

// print vtcm scratch buffer
void print_vtcm_scratch_16(void)
{
	unsigned short *vtmp = (unsigned short *)VTCM_BASE_ADDRESS;
    int i, j;

	printf("\n\nPrinting the vtcm scratch in half words");

	for(i = 0; i < SCRATCH_SIZE; i++)
	{
		if ((i % MATRIX_SIZE) == 0) printf("\n");
		for(j = 0; j < 2; j++)
			printf("%c", (char)((vtmp[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// print vtcm scratch buffer
void print_vtcm_scratch_32(void)
{
	unsigned int *vtmp = (unsigned int *)VTCM_BASE_ADDRESS;
    int i, j;

	printf("\n\nPrinting the vtcm scratch in words");

	for(i = 0; i < SCRATCH_SIZE; i++)
	{
		if ((i % MATRIX_SIZE) == 0) printf("\n");
		for(j = 0; j < 4; j++)
			printf("%c", (char)((vtmp[i] >> j*8) & 0xff));

		printf(" ");
	}
}


// create byte offsets to be a diagonal of the matrix with 16 bit elements
void create_offsets_and_values_16(void)
{
	unsigned short half_element = 0;
	int i, j;
	char letter = 'A';

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		half_offsets[i]	= i*(2*MATRIX_SIZE + 2);

		half_element = 0;
		for(j = 0; j < 2; j++)
			half_element |= letter << j*8;

		half_values[i] = half_element;

		letter++;
		// reset to 'A'
		if(letter == 91) letter = 65;
	}
}

// create byte offsets to be a diagonal of the matrix with 32 bit elements
void create_offsets_and_values_32(void)
{
	unsigned int word_element = 0;
	int i, j;
	char letter = 'A';

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		word_offsets[i]	= i*(4*MATRIX_SIZE + 4);

		word_element = 0;
		for(j = 0; j < 4; j++)
			word_element |= letter << j*8;

		word_values[i] = word_element;

		letter++;
		// reset to 'A'
		if(letter == 91) letter = 65;
	}
}

// create byte offsets to be a diagonal of the matrix with 16 bit elements and 32 bit offsets
void create_offsets_and_values_16_32(void)
{
	unsigned int half_element = 0;
	int i, j;
	char letter = 'A';

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		word_offsets[i]	= i*(2*MATRIX_SIZE + 2);

		half_element = 0;
		for(j = 0; j < 2; j++)
			half_element |= letter << j*8;

		half_values[i] = half_element;

		letter++;
		// reset to 'A'
		if(letter == 91) letter = 65;
	}
}

#define SCATTER_RELEASE(ADDR) asm volatile("vmem(%0 + #0):scatter_release\n" :: "r" (ADDR) );
// scatter the 16 bit elements using intrinsics
void vector_scatter_16(void)
{
	START_CYCLES;

	// copy the offsets and values to vectors
	HVX_Vector offsets	= *(HVX_Vector *)half_offsets;
	HVX_Vector values	= *(HVX_Vector *)half_values;

	// do the scatter
	Q6_vscatter_RMVhV(VTCM_SCATTER16_ADDRESS, region_len, offsets, values);

#if SYNC_VECTOR
    // do the sync operation
	SCATTER_RELEASE(vscatter16);
	// This dummy load from vscatter16 is to complete the synchronization.
	// Normally this load would be deferred as long as possible to minimize stalls.
	volatile HVX_Vector vDummy = *(HVX_Vector *)vscatter16;
#endif

	PRINT_CYCLES("\nVector Scatter 16 cycles = %llu\n");
}

// scatter the 32 bit elements using intrinsics
void vector_scatter_32(void)
{
	START_CYCLES;

	// copy the offsets and values to vectors
	HVX_Vector offsetslo	= *(HVX_Vector *)word_offsets;
	HVX_Vector offsetshi	= *(HVX_Vector *)&word_offsets[MATRIX_SIZE/2];
	HVX_Vector valueslo		= *(HVX_Vector *)word_values;
	HVX_Vector valueshi		= *(HVX_Vector *)&word_values[MATRIX_SIZE/2];

	// do the scatter
	Q6_vscatter_RMVwV(VTCM_SCATTER32_ADDRESS, region_len, offsetslo, valueslo);
	Q6_vscatter_RMVwV(VTCM_SCATTER32_ADDRESS, region_len, offsetshi, valueshi);

#if SYNC_VECTOR
    // do the sync operation
	SCATTER_RELEASE(vscatter32);
	// This dummy load from vscatter32 is to complete the synchronization.
	// Normally this load would be deferred as long as possible to minimize stalls.
	volatile HVX_Vector vDummy = *(HVX_Vector *)vscatter32;
#endif

	PRINT_CYCLES("\nVector Scatter 32 cycles = %llu\n");
}

// scatter the 16 bit elements with 32 bit offsets using intrinsics
void vector_scatter_16_32(void)
{
	START_CYCLES;

	// get the word offsets in a vector pair
	HVX_VectorPair offsets	= *(HVX_VectorPair *)word_offsets;

	// these values need to be shuffled for the RMWwV scatter
	HVX_Vector values	= *(HVX_Vector *)half_values;
	values				= Q6_Vh_vshuff_Vh(values);

	// do the scatter
	Q6_vscatter_RMWwV(VTCM_SCATTER16_32_ADDRESS, region_len, offsets, values);

#if SYNC_VECTOR
    // do the sync operation
	SCATTER_RELEASE(vscatter16_32);
	// This dummy load from vscatter16_32 is to complete the synchronization.
	// Normally this load would be deferred as long as possible to minimize stalls.
	volatile HVX_Vector vDummy = *(HVX_Vector *)vscatter16_32;
#endif

	PRINT_CYCLES("\nVector Scatter 16_32 cycles = %llu\n");
}



// gather the elements from the scatter16 buffer
void vector_gather_16(void)
{
	START_CYCLES;

	HVX_Vector *vgather	= (HVX_Vector *)VTCM_GATHER16_ADDRESS;
	HVX_Vector offsets	= *(HVX_Vector *)half_offsets;

	// do the gather to the gather16 buffer
	Q6_vgather_ARMVh( vgather, VTCM_SCATTER16_ADDRESS, region_len, offsets );


#if SYNC_VECTOR
    // This dummy read of vgather will stall until completion
	volatile HVX_Vector vDummy = *(HVX_Vector *)vgather;
#endif

	PRINT_CYCLES("\nVector Gather 16 cycles = %llu\n");

}

// gather the elements from the scatter32 buffer
void vector_gather_32(void)
{
	START_CYCLES;

	HVX_Vector *vgatherlo	= (HVX_Vector *)VTCM_GATHER32_ADDRESS;
	HVX_Vector *vgatherhi	= (HVX_Vector *)(VTCM_GATHER32_ADDRESS + (MATRIX_SIZE*2));
	HVX_Vector offsetslo	= *(HVX_Vector *)word_offsets;
	HVX_Vector offsetshi	= *(HVX_Vector *)&word_offsets[MATRIX_SIZE/2];

	// do the gather to vgather
	Q6_vgather_ARMVw( vgatherlo, VTCM_SCATTER32_ADDRESS, region_len, offsetslo );
	Q6_vgather_ARMVw( vgatherhi, VTCM_SCATTER32_ADDRESS, region_len, offsetshi );

#if SYNC_VECTOR
    // This dummy read of vgatherhi will stall until completion
	volatile HVX_Vector vDummy = *(HVX_Vector *)vgatherhi;
#endif

	PRINT_CYCLES("\nVector Gather 32 cycles = %llu\n");
}


// gather the elements from the scatter16_32 buffer
void vector_gather_16_32(void)
{
	START_CYCLES;

	// get the vtcm address to gather from
	HVX_Vector *vgather	= (HVX_Vector *)VTCM_GATHER16_32_ADDRESS;

	// get the word offsets in a vector pair
	HVX_VectorPair offsets	= *(HVX_VectorPair *)word_offsets;

	// do the gather to vgather
	Q6_vgather_ARMWw(vgather, VTCM_SCATTER16_32_ADDRESS, region_len, offsets);

    // the read of gather will stall until completion
	volatile HVX_Vector values = *(HVX_Vector *)vgather;

	// deal the elements to get the order back
	values = Q6_Vh_vdeal_Vh(values);

	// write it back to vtcm address
	*(HVX_Vector *)vgather = values;


	PRINT_CYCLES("\nVector Gather 16_32 cycles = %llu\n");
}



// These scalar functions are the C equivalents of the vector functions that use HVX

// scatter the 16 bit elements using C
void scalar_scatter_16(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vscatter16[half_offsets[i]/2] = half_values[i];
	}

	PRINT_CYCLES("\nScalar Scatter 16 cycles = %llu\n");

}

// scatter the 32 bit elements using C
void scalar_scatter_32(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vscatter32[word_offsets[i]/4] = word_values[i];
	}

	PRINT_CYCLES("\n\nScalar Scatter 32 cycles = %llu\n");
}

// scatter the 32 bit elements using C
void scalar_scatter_16_32(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vscatter16_32[word_offsets[i]/2] = half_values[i];
	}

	PRINT_CYCLES("\n\nScalar Scatter 16_32 cycles = %llu\n");
}


// gather the elements from the scatter buffer using C
void scalar_gather_16(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vgather16[i] = vscatter16[half_offsets[i]/2];
	}

	PRINT_CYCLES("\n\nScalar Gather 16 cycles = %llu\n");
}

// gather the elements from the scatter buffer using C
void scalar_gather_32(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vgather32[i] = vscatter32[word_offsets[i]/4];
	}

	PRINT_CYCLES("\n\nScalar Gather 32 cycles = %llu\n");

}

// gather the elements from the scatter buffer using C
void scalar_gather_16_32(void)
{
	START_CYCLES;
	int i;

	for(i=0; i < MATRIX_SIZE; ++i)
	{
		vgather16_32[i] = vscatter16_32[word_offsets[i]/2];
	}

	PRINT_CYCLES("\n\nScalar Gather 16_32 cycles = %llu\n");

}


// These functions print the buffers to the display

// print scatter16 buffer
void print_scatter16_buffer(void)
{
	printf("\n\nPrinting the 16 bit scatter buffer at 0x%08x", VTCM_SCATTER16_ADDRESS);
	int i, j;

	for(i = 0; i < SCATTER_BUFFER_SIZE; i++)
	{
		if ((i % MATRIX_SIZE) == 0) printf("\n");

		for(j = 0; j < 2; j++)
			printf("%c", (char)((vscatter16[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// print the gather 16 buffer
void print_gather_result_16(void)
{
	printf("\n\nPrinting the 16 bit gather result at 0x%08x\n", VTCM_GATHER16_ADDRESS);
	int i, j;

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		for(j = 0; j < 2; j++)
			printf("%c", (char)((vgather16[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// print the scatter32 buffer
void print_scatter32_buffer(void)
{
	printf("\n\nPrinting the 32 bit scatter buffer at 0x%08x", VTCM_SCATTER32_ADDRESS);
	int i, j;

	for(i = 0; i < SCATTER_BUFFER_SIZE; i++)
	{
		if ((i % MATRIX_SIZE) == 0) printf("\n");

		for(j = 0; j < 4; j++)
			printf("%c", (char)((vscatter32[i] >> j*8) & 0xff));

		printf(" ");
	}
}


// print the gather 32 buffer
void print_gather_result_32(void)
{
	printf("\n\nPrinting the 32 bit gather result at 0x%08x\n", VTCM_GATHER32_ADDRESS);
	int i, j;

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		for(j = 0; j < 4; j++)
			printf("%c", (char)((vgather32[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// print the scatter16_32 buffer
void print_scatter16_32_buffer(void)
{
	printf("\n\nPrinting the 16_32 bit scatter buffer at 0x%08x", VTCM_SCATTER16_32_ADDRESS);
	int i, j;

	for(i = 0; i < SCATTER_BUFFER_SIZE; i++)
	{
		if ((i % MATRIX_SIZE) == 0) printf("\n");

		for(j = 0; j < 2; j++)
			printf("%c", (char)((vscatter16_32[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// print the gather 16_32 buffer
void print_gather_result_16_32(void)
{
	printf("\n\nPrinting the 16_32 bit gather result at 0x%08x\n", VTCM_GATHER16_32_ADDRESS);
	int i, j;

	for(i = 0; i < MATRIX_SIZE; i++)
	{
		for(j = 0; j < 2; j++)
			printf("%c", (char)((vgather16_32[i] >> j*8) & 0xff));

		printf(" ");
	}
}

// set up the tcm address translation
// Note: This method is only for the standalone environment
// SDK users should use the "VTCM Manager" to use VTCM
void setup_tcm(void)
{
	uint64_t pa = VTCM_BASE_ADDRESS;
    void *va = (void *)VTCM_BASE_ADDRESS;

	unsigned int xwru = 15;
	unsigned int cccc = 7; // write back and cacheable
	unsigned int asid = 0;
	unsigned int aa = 0;
	unsigned int vg = 3; // Set valid and ignore asid
	unsigned int page_size2 = 8; // 256KB
	add_translation_extended(1, va, pa, page_size2, xwru, cccc, asid, aa, vg);
	printf("Adding 256KB VTCM Page at VA:%x PA:%llx\n", (int)va, pa);
}


int main()
{
#ifndef REGRESS
#if __HEXAGON_ARCH__ > 66
	printf("\n\nError: There are no v67 cores with an HVX coprocessor\n\n");
	exit(0);
#endif
#endif

    SIM_ACQUIRE_HVX;

#ifdef HVX_DOUBLE
	SIM_SET_HVX_DOUBLE_MODE;
#endif

	setup_tcm();

	prefill_vtcm_scratch();

// 16 bit elements with 16 bit offsets
#if 1
	create_offsets_and_values_16();

#if PRINT_CYCLE_COUNTS
	scalar_scatter_16();
#endif

	vector_scatter_16();

	print_scatter16_buffer();

#if PRINT_CYCLE_COUNTS
	scalar_gather_16();
#endif

	vector_gather_16();

	print_gather_result_16();

#endif


// 32 bit elements with 32 bit offsets
#if 1
	create_offsets_and_values_32();

#if PRINT_CYCLE_COUNTS
	scalar_scatter_32();
#endif

	vector_scatter_32();

	print_scatter32_buffer();

#if PRINT_CYCLE_COUNTS
	scalar_gather_32();
#endif

	vector_gather_32();

	print_gather_result_32();

#endif

// 16 bit elements with 32 bit offsets
#if 1
	create_offsets_and_values_16_32();

#if PRINT_CYCLE_COUNTS
	scalar_scatter_16_32();
#endif

	vector_scatter_16_32();

	print_scatter16_32_buffer();

#if PRINT_CYCLE_COUNTS
	scalar_gather_16_32();
#endif

	vector_gather_16_32();

	print_gather_result_16_32();

#endif



	printf("\n\n");

    return 0;
}

