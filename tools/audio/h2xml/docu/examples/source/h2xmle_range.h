/**************************************************************************//**
* @file
* Example for h2xmle_range
*
* If name is not speciefied, the variabel name is used as name
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a;	/**<		@h2xmle_range {0..3} 	*/	
	int b;   			/**<		@h2xmle_range {-10..20} 	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				/**<		@h2xmle_range {5..20} @h2xmle_default {10}*/
	long b;   			/**<		@h2xmle_range {-5..20} */
};

/** @} */							/* End of Module */						

