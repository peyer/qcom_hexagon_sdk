/**************************************************************************//**
* @file
* Example h2xmlm_createParameter 
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
   	@h2xmlm_InputPins 				{In1=1;In2=2}							
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}					  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 				{Parameter0,0x11111188} */
struct param_1{
	short 	a1;		/**< @h2xmle_default		{0x33} */
	int 	b1;   	/**< @h2xmle_default		{0x44} */	
	#ifdef __H2XML__
	int emptyArray[0];
	#endif
};


/** @} */							/* End of Module */						
