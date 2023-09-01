	/**************************************************************************************************
	* Copyright (c) Date: Sep 2013 Qualcomm Technologies INCORPORATED                                 *
	* All Rights Reserved                                                                             *
	***************************************************************************************************/

   .file       "dspCV_sync.S"

	/*================================================================================================*/
	/*  FUNCTION     : dspCV_dec_refcnt_and_nullify_ptr                                               */
	/*================================================================================================*/
	/*  PARAMETERS   :                                                                                */
	/*  ============                                                                                  */
	/* R0 : pointer to dspCV_syncobj_t  (struct of ref count & any pointer)                           */
	/*================================================================================================*/
	/***************************************************************************************************
	* IMPLEMENTATION SUMMARY                                                                           *
	* ======================                                                                           *
	*   Atomically decrements ref count and sets pointer to NULL if ref count is reaching 0.           *
    *   Returns updated ref count                                                                      *
	***************************************************************************************************/
    
   .text
   .p2align 4						   // ensures 16-byte alignment of first packet
   .globl dspCV_dec_refcnt_and_nullify_ptr// makes function have global scope
   .type       dspCV_dec_refcnt_and_nullify_ptr, @function
dspCV_dec_refcnt_and_nullify_ptr:
    {
      R3:2 = MEMD_LOCKED(R0)
    }{
      P0 = CMP.EQ(R2,#1)
      R2 = ADD(R2,#-1)                 // decrement ref count
      IF (P0.NEW) R3 = #0              // if decrementing ref count to 0, set ptr = NULL
    }{
      MEMD_LOCKED(R0,P0) = R3:2
    }{
      IF (!P0) JUMP dspCV_dec_refcnt_and_nullify_ptr
    }{
      R0 = R2
      JUMPR R31
    }
   .size       dspCV_dec_refcnt_and_nullify_ptr, .-dspCV_dec_refcnt_and_nullify_ptr
