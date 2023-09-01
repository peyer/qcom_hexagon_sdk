/**************************************************************************//**
* @file
* Example for h2xmlp_emptyParameter 
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
   	@h2xmlm_InputPins 				{In1=1;In2=2}							
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}					  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 				{Parameter0,0x11111188} */
struct param_0{
	short 	a1;		/**< @h2xmle_default		{0x33} */
	int 	b1;   	/**< @h2xmle_default		{0x44} */	
};

/**
	@h2xmlp_emptyParameter 			{EmptyParameter1, 0x12345678} 
	@h2xmlp_description				{Description empty Parameter1 Module 1}
*/

/**
	@h2xmlp_emptyParameter 			{EmptyParameter2, 0x87654321} 
	@h2xmlp_description				{Description empty Parameter2 Module 1}
*/

/** @h2xmlp_parameter 				{Parameter3,0x11111199} */
struct param_1{
	short 	a1;		/**< @h2xmle_default		{0x33} */
	int 	b1;   	/**< @h2xmle_default		{0x44} */	
};

/** @} */							/* End of Module */						
