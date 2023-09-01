/**************************************************************************//**
* @file
* Example for h2xmle_variableArraySize
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned short knElements;		
	int b[0];   				
	/**<
		@h2xmle_variableArraySize {knElements} 	
	*/
};



/** @} */							/* End of Module */						

