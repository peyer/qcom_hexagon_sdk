/**************************************************************************//**
* @file
* Example for h2xmlp_version
******************************************************************************/

#define PARAM1_VERSION	1

enum PVERsion {
	PARAM2_VERSION = 2,
	PARAM3_VERSION = 3
};

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
	@h2xmlp_version		{PARAM1_VERSION}
*/
struct param_1{
	unsigned short a;
	unsigned int b;   	
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmlp_version		{PARAM2_VERSION}

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

/** @h2xmlp_parameter 	{Parameter3,0x33333399} 
	@h2xmlp_version		{PARAM3_VERSION}
*/
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
	@h2xmlp_version		{0x4}
*/

/** @} */							/* End of Module */						
