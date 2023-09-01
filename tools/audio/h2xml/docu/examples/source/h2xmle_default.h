/**************************************************************************//**
* @file
* Example for h2xmle_default
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a;	/**<		@h2xmle_default {0x1122} 		*/	
	int b;   			/**<		@h2xmle_default {-0x17654321} 	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				/**<		@h2xmle_default {0x44} 		*/
	long b;   			/**<		@h2xmle_default {0x17654321} 	*/
};

/** @} */							/* End of Module */						

