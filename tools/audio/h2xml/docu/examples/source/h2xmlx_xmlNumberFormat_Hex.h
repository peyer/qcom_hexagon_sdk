/**************************************************************************//**
* @file
* Example for h2xmlx_xmlNumberFormat {HEX}
******************************************************************************/

/**
	@h2xml_globalAnnotations		{true}	
 	@h2xmlx_xmlNumberFormat			{hex}	
 	@h2xml_globalAnnotations		{false}
*/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
 	@h2xmlm_InputPins 				{In1=1;In2=0x2}							
   	@h2xmlm_OutputPins 				{Out1=0x3;Out2=4}				
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 			{Parameter1,0x11111111} */
struct param_1{		
	long a;	
	int b;							///< @h2xmle_default {-10}
	unsigned short c;				///< @h2xmle_default {0xa}							
};

/** @} */							/* End of Module */							