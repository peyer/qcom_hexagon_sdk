/**************************************************************************//**
* @file
* Example for h2xmle_range
*******************************************************************************/

typedef struct {
	int s1a;
	int s1b;			/**<		@h2xmle_name {s1bName} */ 		
	int s1c;
}sStruct1_t;



/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short a;	/**<		@h2xmle_name {name1} 	*/	
	sStruct1_t x;		/**<		@h2xmle_name {struct1} 	*/ 
	unsigned int b;   	/**<		@h2xmle_name {name2} 	*/
	sStruct1_t y;		/**<							 	*/ 
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				/**<		@h2xmle_name {name3} */
	long b;   	
};

/** @} */							/* End of Module */						

