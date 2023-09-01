/**************************************************************************//**
* @file
* test #ifdef within parameter
******************************************************************************/
#define TEST_DEFINE

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter {Parameter1,0x11111199} */
struct param_1{
	
	unsigned short a;
	#ifdef TEST_DEFINE
	unsigned int b;  
	#else
	unsigned int c;  
	#endif 	
	unsigned short d;
	#ifdef TEST_DEFINE2
	unsigned int e;  
	#else
	unsigned int f;  
	#endif 	
};

/** @h2xmlp_parameter {Parameter2,0x22222299} */
struct param_2{
	char a;
	#ifndef TEST_DEFINE
	long b;   
	#else
	long c;
	#endif	
	char d;
	#ifndef TEST_DEFINE2
	long e;   
	#else
	long f;
	#endif	
};

/** @} */							/* End of Module */						

