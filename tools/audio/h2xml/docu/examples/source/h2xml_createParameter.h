/**************************************************************************//**
* @file
* Example h2xml_createParameter 
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
	@h2xml_createParameter														<!-- creates an empty structure -->
    @h2xmlm_InsertParameter														<!-- inserts this empty strucure into the module's parameter list -->
	@h2xmlp_parameter 				{EmptyParameter1, 0x12345678} 				<!-- define parameter properties for this structure -->
	@h2xmlp_description				{Description empty Parameter1 Module 1}
*/


/** @h2xmlp_parameter 				{Parameter3,0x11111199} */
struct param_1{
	short 	a1;		/**< @h2xmle_default		{0x33} */
	int 	b1;   	/**< @h2xmle_default		{0x44} */	
};

/** @} */							/* End of Module */						
