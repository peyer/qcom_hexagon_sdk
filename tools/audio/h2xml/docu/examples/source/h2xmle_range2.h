/**************************************************************************//**
* @file
* Example for h2xmle_range
* especially tests if hex values are correctly sign extended
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	short a;			/**<		@h2xmle_range {-10..3} 			@h2xmle_default {0xffff} */	
	int b;   			/**<		@h2xmle_range {0xfffffff8..20} 	@h2xmle_default {-4}     */
	unsigned short c;	/**<		@h2xmle_range {0xfff8..0xffff} 	@h2xmle_default {65529} */	
	short d;			/**<		@h2xmle_rangeList {value1=0xfff8; value2=1; value3=3} 	@h2xmle_default {-8}*/	
};

/** @} */							/* End of Module */						

