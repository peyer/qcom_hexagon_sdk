/**************************************************************************//**
* @file
* Example for h2xmle_rangeList
* Range errors are introduced, no output XML is generated. See output log file for errors
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a;	/**<		@h2xmle_rangeList {value1=3;value2=1;value3=3} 	*/			// Error: Default Value '0' not in Range List
	unsigned short b;   /**<		@h2xmle_rangeList {test1=-1; test2=0x7fffffff } 	*/		// Error: List value '0x7fffffff' > type range '65535'

	char c;				
	/**<		
		@h2xmle_rangeList 	{a=-1; b=0x20} 
		@h2xmle_default 	{5} 
	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		@h2xmle_rangeList {	enable1=4; 
									enable2=1;
									enable3=3;
									enable4=500
									}	
	*/
	unsigned long b;   			/**<		@h2xmle_rangeList {a=-1; b=0x20; c=0x0} */
};

/** @} */							/* End of Module */						

