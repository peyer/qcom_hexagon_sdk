/**************************************************************************//**
* @file
* Example for h2xmlp_deprecated
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }	
 	@h2xmlm_deprecated						  
   	@{
----------------------------------------------------------------------------*/
/** 
	@h2xmlp_parameter {Parameter1,0x11111199} 
	@h2xmlp_deprecated
*/
struct param_1{
	unsigned short a;
	unsigned int b;   	
};

/** 
	@h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmlp_deprecated  {true}
*/
struct param_2{
	char a;
	long b;   	
};

/** @} */							/* End of Module */						

