/**************************************************************************//**
* @file
* Example for h2xmle_rangeList
******************************************************************************/
#define NUMM 2

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,NUMM} 
*/
struct param_1{
	unsigned short a;	/**<		@h2xmle_rangeList {value1=0;value2=1;value3=3} 	*/	
	int b;   			/**<		@h2xmle_rangeList {test1=0; test2=1}  	*/
	char c;				
	/**<		
		@h2xmle_rangeList 	{a=-1; b=0x20} 
		@h2xmle_default 	{-1} 
	*/
	int d[NUMM];
	/**< 
		@h2xmle_rangeList   {"DISABLE"=0;
                            "ENABLE"=1}  
	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		@h2xmle_rangeList {	enable1=4; 
									enable2=1;
									enable3=3;
									enable4=55
									}	
				@h2xmle_default {55} 
	*/
	long b;   			/**<		@h2xmle_rangeList {a=-1; b=0x20; c=0x0} */
};

/** @} */							/* End of Module */						

