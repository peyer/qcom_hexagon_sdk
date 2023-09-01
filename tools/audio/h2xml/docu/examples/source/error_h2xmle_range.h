/**************************************************************************//**
* @file
* Example for h2xmle_range
*
* Error messages are generated (see log file), no XML file is produced
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a[20];	/**<		@h2xmle_range {-1..0x7fffffff} 	*/	
	int b;   			/**<		@h2xmle_range {10..20} 	*/
	char c;				/**<		@h2xmle_range {2..1000} */
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				/**<		@h2xmle_range {5..20}	@h2xmle_default {100} */
	long b;   			/**<		@h2xmle_range {-10..-5} */
};

/** @} */							/* End of Module */						

