/**************************************************************************//**
* @file
* Example for __attribute__ ((packed))
*******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	char a;					
	short b;		
	long c;   
	union {
		char x1;
		struct {
			char x2;
			long x3;
		}s1;
	}u1;
} __attribute__ ((packed));

/** @} */							/* End of Module */						
