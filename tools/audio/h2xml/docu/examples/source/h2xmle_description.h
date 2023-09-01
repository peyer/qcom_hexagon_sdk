/**************************************************************************//**
* @file
* Example for h2xmle_description
*******************************************************************************/

typedef struct {
	int s1a;
	int s1b;			
	/**<		
		@h2xmle_description {This is element s1b. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 		
	int s1c;
}sStruct1_t;



/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a;	
	/**<		
		@h2xmle_description {This is element a. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 			
	sStruct1_t x;	
	/**<		
		@h2xmle_description {This is struct x. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 	 
	unsigned int b; 
	/**<		
		@h2xmle_description {This is element b. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		
		@h2xmle_description {This is element a. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 
	
	/**		
		@h2xmle_description {This is element b. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
	*/ 
	long b;   	
};

/** @} */							/* End of Module */						

