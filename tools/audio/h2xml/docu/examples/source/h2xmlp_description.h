/**************************************************************************//**
* @file
* Example for h2xmlp_description
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
	@h2xmlp_description	{
	Description of Parameter 1. Description continued.\n
	Description continued.\n Description continued.\n
	Description continued.\n
	}
*/
struct param_1{
	unsigned short a;
	unsigned int b;   	
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmlp_description	{
	Description of Parameter 2. Description continued.\n
	Description continued.\n Description continued.\n
	Description continued.\n}
*/
struct param_2{
	char a;
	long b;   	
};

/** @} */							/* End of Module */						


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_2,0x22222222  }				  
   	@{								
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter3,0x33333399} */
struct param_3{
	unsigned short a;
	int b;   	
};

/** @h2xmlp_parameter 	{Parameter4,0x44444499} */
struct param_4{
	unsigned char a;
	long b;   	
};
/**<
	@h2xmlp_description	{
	Description of Parameter 4. Description continued.\n
	Description continued.\n Description continued.\n
	Description continued.\n
	}
*/

/** @} */							/* End of Module */						
