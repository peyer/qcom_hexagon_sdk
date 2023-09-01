/*===========================================================================
                   QNX PMEM Lite Version
DESCRIPTION
  This file declares the functions exported by the PMEM driver.

EXTERNALIZED FUNCTIONS
  The functions exported are pmem_malloc, pmem_free and pmem_get_phys_addr

INITIALIZATION AND SEQUENCING REQUIREMENTS
      
  Copyright (c) 2011-2018 by QUALCOMM, Incorporated.  All Rights Reserved.
===========================================================================*/

/*===========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/29/14   ranc    Added PMEM_DMOV_ID
05/15/13   ranc    Added PMEM_OCMEM_ID
11/15/11   rohitn  Added API for shared memory support
02/23/11   kalyanr Full fledged initial version.
10/19/10   bkchan  Initial Version
===========================================================================*/

/*===========================================================================
                     INCLUDE FILES FOR MODULE
===========================================================================*/
#ifndef QNX_PMEM_H
#define QNX_PMEM_H
#if __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <pmem_id.h>
#define IN 
#define OUT

#ifndef PMEM_GET_PHYS_ADDR
#define PMEM_GET_PHYS_ADDR(x) pmem_get_phys_addr(x)
#endif

/* Allocate physically non contigous memory.
 * If the DMA device has a S1 SMMU, then user should allocate 
 * non-contiguous memory to reduce memory pressure during 
 * low memory situations. */
#define PMEM_FLAGS_PHYS_NON_CONTIG        0x00000010

/* Allocate physically contiguous memory (default).
 * If the DMA device doesn't have S1 SMMU, or if device is configured
 * for pass-through S1 SMMU, then user should allocate contiguous memory.*/
#define PMEM_FLAGS_PHYS_CONTIG            0x00000000

/* Create a shared memory file-descriptor that represents the mappings. 
 * The fd can be obtained via "pmem_get_fd()". 
 * The fd can then be used to map the buffer into another process 
 * using "mmap64_peer()". */
#define PMEM_FLAGS_SHMEM                  0x00000040

/* Deprecated, shouldn't be used, marked for future removal. 
 * Users should instead configure device tree to route allocations to a 
 * typed memory pool.*/
#define PMEM_FLAGS_TYPED_MEM              0x10000000

/* Deprecated, shouldn't be used, marked for future removal*/
#define PMEM_FLAGS_PPGA                   0x00000020

/* Allocate uncached memory (default).*/
#define PMEM_FLAGS_CACHE_NONE             0x00000000

/* Allocate write-back, write-allocate memory, to be used if IP block is 
 * coherent with CPU cache. */
#define PMEM_FLAGS_CACHE_WB_WA            0x00000100 

/* Allocate write-back, no write-allocate memory, to be used if IP block is 
 * coherent with CPU cache. 
 * Supported only if PMEM_FLAGS_SHMEM is set.*/
#define PMEM_FLAGS_CACHE_WB_NWA           0x00000200

/* Allocate write through, no write allocate memory, to be used if SW makes 
 * frequent writes to the memory, example SW rendering.  
 * Supported only if PMEM_FLAGS_SHMEM is set.*/
#define PMEM_FLAGS_CACHE_WT_NWA           0x00000400
#define PMEM_FLAGS_CACHE_MASK             0x00000F00

/* Alignment flags */
#define PMEM_ALIGNMENT_4K       12
#define PMEM_ALIGNMENT_8K       13
#define PMEM_ALIGNMENT_16K      14
#define PMEM_ALIGNMENT_32K      15
#define PMEM_ALIGNMENT_64K      16
#define PMEM_ALIGNMENT_128K     17
#define PMEM_ALIGNMENT_256K     18
#define PMEM_ALIGNMENT_512K     19
#define PMEM_ALIGNMENT_1M       20
#define PMEM_ALIGNMENT_2M       21

#define PMEM_PAGESIZE          (1U << PMEM_ALIGNMENT_4K) 
#define PMEM_PAGESHIFT         (PMEM_ALIGNMENT_4K) 
/*===========================================================================
*                         CONSTANT DEFINITIONS
============================================================================*/

typedef struct pmem_rgn_s  {
    /* Size of this region */
    uint32_t size;

    /* Physical address */
    uint64_t paddr;
}pmem_rgn_t;

typedef struct _pmem_rgn_tbl_s {
    /* Number of regions in this buffer */
    unsigned nrgns;

    struct pmem_rgn_s rgn [ 0 ] ;
}__attribute__ ((packed)) pmem_rgn_tbl_t;

struct _pmem_handle_entry_s ;
typedef struct _pmem_handle_entry_s* pmem_handle_t;

/*===========================================================================
                 Includes and Public Data Declarations
===========================================================================*/
/**===========================================================================

FUNCTION pmem_init

@brief  This function needs to be called by the user before any other 
        pmem_* APIs are called. This function initializes the pmem library.
        It parses the device tree to gather typed memory pool bindings.
        This function should be called if application needs pmem_mallocXXX()
        calls after pmem_deinit() is called.

@dependencies
  None

@return
   0 on success, non-zero on failure.

@sideeffects

==========================================================================*/
int pmem_init(void);
/**========================================================================

FUNCTION pmem_deinit

@brief  This function cleans up any memory resources allocated in the pmem 
        library.  Should only be called when the application exits.

@dependencies
  None

@return
   0 on success, non-zero on failure.

@sideeffects

  frees all memory allocated by pmem client. Cannot call pmem_mallocXXX 
  calls after this.  Need to call pmem_init() again.

==========================================================================*/
int pmem_deinit(void);

/**=========================================================================

FUNCTION pmem_malloc_ext_v2

@brief  This function allocates physical memory with size rounded up to the nearest
        alignment specified.The allocated physical memory can be contigous or non-congituous
        depending on the flags passed.

@param [in] size  Size of memory chunk to be allocated, Will be rounded up to 4K size
@param [in] id    PMEM id for the allocation, see <amss/pmem_id.h>
@param [in] flags flags on the type of allocation, see PMEM_FLAGS_* for 
                  descriptions. flags can be OORed together to yield the 
                  desired effect. 

@param [in] alignment  alignment in the power of 2. 
                  Minimun alignment is 4K (i.e value 12). See different 
                  supported values under PMEM_ALIGMENT_XXX definitions.

@param [in] vmid_mask vmid_mask of the GVM(s) for which this allocation is 
                 being made.  "vmid" is defined in device tree, 
                 (see property "vmid"). 
                 The vmid_mask can be obtained by (1u << vmid). 
                 vmid_mask of zero is a hint for local allocations.

@param [out] handlep  The output opaque pmem handle associated with this 
                allocation. PMEM handles are unique throughout the system
                and users can pass them around different processes.

@param [rsv] rsv1 Reserved for future expansion.
@dependencies
None

@return
Returns virtual pointer to the allocated memory (void*)

@sideeffects

Initializes pmem library when this call is first invoked.

==========================================================================*/
void* pmem_malloc_ext_v2
(
 uint32_t size, 
 uint32_t id, 
 uint32_t flags, 
 uint32_t alignment, 
 uint32_t vmid_mask , 
 pmem_handle_t * OUT handlep, 
 void * IN rsv1
 );

/**=========================================================================
FUNCTION pmem_malloc_ext 

Turns around to calls pmem_malloc_ext_v2().
See pmem_malloc_ext_v2() for parameter and return description.

"flags" is updated to create a shared memory object.
"vmid_mask" is set to zero.
"pmem handle" is not created.
==========================================================================*/
void* pmem_malloc_ext
(
 uint32_t size, 
 uint32_t id, 
 uint32_t flags, 
 uint32_t alignment
);

/**=========================================================================
FUNCTION pmem_malloc

Turns around to calls pmem_malloc_ext_v2().
See pmem_malloc_ext_v2() for parameter and return description.

"flags" is set to allocate contiguous, uncached, shared memory object.
"vmid_mask" is set to zero.
"pmem handle" is not created.
==========================================================================*/
void* pmem_malloc
(
 uint32_t size, /* Size of the memory chunk to be allocated */
 uint32_t id    /* PMEM client identifier */
);

/**=========================================================================

FUNCTION pmem_free

@brief  This function frees the previously allocated address. 
        The previous allocation can either be from pmem_malloc() or 
        pmem_malloc_ext() or pmem_malloc_ext_v2().
        It also frees up the pmem_handle_t if it was previously allocated by
        pmem_malloc_ext_v2().

@param [in] ptr  address of memory previously allocated by pmem_malloc or
        pmem_malloc_ext or pmem_malloc_ext_v2().

@dependencies
  None

@return
  Returns 0 on success , non-zero value on failure.

@sideeffects
  None

==========================================================================*/
int pmem_free
(
    void* IN vaddr /* Memory address to be freed*/
);

/**=========================================================================

FUNCTION pmem_get_phys_addr

@brief This function translates a given virtual address to its equivalent 
       physical address. If a buffer is dis-contiguous, users should 
       directly use "mem_offset64()" to walk the physical pages.


@param [in] ptr  address of memory previously allocated by pmem_malloc or 
        pmem_malloc_ext or pmem_malloc_ext_v2

@dependencies
  None

@return
  Returns Physical Address

@sideeffects
  None

==========================================================================*/
void* pmem_get_phys_addr
(
    void* IN ptr /* Virtual Address */
);

/**=========================================================================

FUNCTION pmem_get_rgn_tbl

@brief This function returns a list of discontiguous physical memory chunks associated with input pmem handle.

@param [in]  PMEM Handle 
@param [out] ptr buffer to hold the pmem region table 

@dependencies
  None

@return
  Returns 0 on success, -1 otherwise 

@sideeffects
 This function allocates memory to hold the region table. 
 Once the region table has been consumed it should be freed by the user
 using free() so as to prevent memory leaks.

==========================================================================*/
int pmem_get_rgn_tbl
(
 const pmem_handle_t IN handle, 
 pmem_rgn_tbl_t** OUT rgn_tblp
);

/**=========================================================================

FUNCTION pmem_get_fd

@brief
This function translates a given virtual address to its equivalent shared memory object file des and its offset

@param [in] ptr  address of memory previously allocated by pmem_malloc or pmem_malloc_ext

@param [in] id   pmem ID of the MM component, this should be the same as the ID with which the buffer was allocated with.

@param [out] ptr  fd of the mapping

@param [out] ptr  offset into the the fd.

@dependencies
None

@return 0 on success, -1 otherwise

@sideeffects
If the memory was originally allocated from a typed memory pool, then there
is no shared memory object, hence no file-descriptor to return.  
In such cases, this function creates a shared memory object and overlays it
on top of the memory regions associated with the input virtual address.
It does this via repeated "lseek() + shm_ctl_special(,SHMCTL_PHYS,) calls.
The calling process must have "PROCMGR_AID_MEM_PHYS" ability on the memory
regions.

==========================================================================*/
int pmem_get_fd
(
 void* IN vaddr,      /*Virtual Address */
 uint32_t IN id,      /*PMEM ID of the MM component*/
 int OUT *fd,          /*Filedes of the shm object*/
 off64_t OUT *offset   /*Offset into the shm object*/
);

/**=========================================================================

FUNCTION pmem_alloc_handle

@brief
This function creates a PMEM handle out of an input rgn table.

@param [in] vmid  VMID of the GVM that owns the rgn table.

@param [in] rgn_tbl  Pointer to the rgn table. 

@param [out] Pointer to PMEM handle

@dependencies
None

@return 0 on success, -1 otherwise

@notes.  
Creating remote handles requires the calling process to have the 
"/pmem/remote-handle" dynamic procmgr ability.
==========================================================================*/

int pmem_alloc_handle 
( 
 int IN vmid, 
 const pmem_rgn_tbl_t * const IN rgn_tbl , 
 pmem_handle_t * OUT handlep
 );

/**========================================================================

FUNCTION pmem_free_handle

@brief
This function frees a pmem handle that was previously allocated.
PMEM handles can be only freed by the process that created it.

@param [in] PMEM handle

@dependencies
None

@return 0 on success, -1 otherwise

==========================================================================*/

int pmem_free_handle 
( 
 const pmem_handle_t IN handle 
 ) ;

/**=========================================================================

FUNCTION pmem_map_handle

@brief
This function maps the physical pages represented by the PMEM handle into 
the process. 

@param [in] PMEM handle

@param [out] Size of the mapping.

@dependencies
None

@return virtual address on success, NULL otherwise.

@notes.  
The calling process must have "/pmem/map-handle" dynamic procmgr ability.
The 'asinfo' region from which the memory pages associated with the PMEM handle 
must be marked as "mappable" in the device tree via the special psuedo PMEM ID 
of "PMEM_MAPPABLE_ID". 

==========================================================================*/

void * pmem_map_handle 
( 
 const pmem_handle_t IN handle , 
 uint32_t * OUT size 
 );

/**=========================================================================

FUNCTION pmem_unmap_handle

@brief
This function unmaps the virtual address previously mapped by pmem_map_handle

@param [in] PMEM handle

@param [in] Virtual address.

@dependencies
None

@return 0 on success.

@note
==========================================================================*/

int pmem_unmap_handle 
( 
 const pmem_handle_t IN handle , 
 void * IN vaddr 
 );

/**=========================================================================

FUNCTION pmem_get_rgn_tbl_extent

@brief
This function returns the extent a pmem region table.

@param [in] Pointer to the pmem region table.

@return Cumulative sizes of chunks within the rgn table.

==========================================================================*/
static inline uint32_t pmem_get_rgn_tbl_extent 
( 
 const pmem_rgn_tbl_t * IN const rgn_tbl 
 ) 
{
    uint32_t extent = 0 ;
    for ( unsigned i = 0 ; i < rgn_tbl->nrgns ; i++ ) {
        extent += rgn_tbl->rgn[i].size;
    }
    return extent;
}

/**=========================================================================

FUNCTION pmem_get_rgn_tbl_size

@brief
This function returns the size of the table.

@param [in] Pointer to the pmem region table.

@return Size of the table.

==========================================================================*/
static inline uint32_t pmem_get_rgn_tbl_size
( 
 const pmem_rgn_tbl_t * IN const rgn_tbl 
 ) 
{
    return rgn_tbl->nrgns * sizeof (pmem_rgn_t) + sizeof (pmem_rgn_tbl_t);
}

/**=========================================================================

FUNCTION pmem_set_handle_property.

@brief
This function sets arbitrary property to a PMEM handle.
The property can be queried at a later stage via "pmem_get_handle_property()".

@param [in] handle PMEM handle.
@param [in] id     PMEM id association, see <amss/pmem_id.h>
@param [in] len    Length of the property, maximum allowed value is 64 bytes.
@param [in] prop   Address of the property.

@return Zero on success.

==========================================================================*/
int pmem_set_handle_property
( 
 const pmem_handle_t handle,
 uint32_t id ,
 uint32_t len ,
 uintptr_t prop_addr
 ) ;

/**=========================================================================

FUNCTION pmem_get_handle_property.

@brief
This function fills the user supplied buffer with PMEM handle's property.
The property has to be 'set' earlier via "pmem_set_handle_property()".

@param [in] handle       PMEM handle.
@param [in] id           PMEM id association, see <amss/pmem_id.h>
@param [in] len          The maximum number of bytes that the server can write at 'prop_addr'
@param [in] prop_addr    The buffer address at which the server needs to copy the property.
                         An error is returned if this user-provided buffer has insufficient space 
                         for storing the property being retrieved.

@return Zero on success.

==========================================================================*/
int pmem_get_handle_property
( 
 const pmem_handle_t handle,
 uint32_t id ,
 uint32_t len ,
 uintptr_t prop_addr 
 ) ;

/**=========================================================================

FUNCTION pmem_allow_nonroot_heap_access

@brief
The start/end addresses of some of PMEM pools may not be known during image creation.
This function allows a non-root process to have access to such dynamically allocated PMEM pool.
It queries the system page to find the memory range of the pool and then calls 
"procmgr_ability()" to add PROCMGR_AID_MEM_PHYS ranges to the process ability list.
The pool bindings are located in the device tree and are instantiated in startup. 
This call needs to be made before the process "drops" root and the process is
then expected to "lock" any further changes to it's abilities via a call to "procmgr_ability()".

@param [in] id           PMEM id association, see <amss/pmem_id.h>
@return Zero on success. 

@dependencies. 
The "mem_phys" ability needs to be "unlocked" for the call to succeed.
The calling process needs to have the "able_priv" ability as part of it's "root" domain.

==========================================================================*/
int pmem_allow_nonroot_heap_access ( uint32_t pmem_id ) ;
#if __cplusplus
}

#endif
#endif /* #ifndef QNX_PMEM_H */
