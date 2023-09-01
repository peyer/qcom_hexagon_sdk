/**************************************************************************//**
* @file
* Several Modules in one file
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
   	@h2xmlm_InputPins 				{In1=1;In2=2}							
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}					  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter {Parameter1,0x11111199} */
struct param_1{
	short a;
	/**<
		@h2xmle_default		{0x11}
	*/
	int b;   
    /**<
 		@h2xmle_default		{0x22}
	*/	
};

/** @} */							/* End of Module */						


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_2,0x22222222  }
   	@h2xmlm_InputPins 				{In1=2;In2=3}							
   	@h2xmlm_OutputPins 				{Out1=4;Out2=5}					  
   	@{								
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter {Parameter1,0x22222299} */
struct param_2{
	int a;
	/**<
		@h2xmle_default		{0x33}
	*/
	short b;   
    /**<
 		@h2xmle_default		{0x44}
	*/	
};

/** @} */							/* End of Module */						
