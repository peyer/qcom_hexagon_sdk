/**************************************************************************//**
* @file
* various XML output properties
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
 	@h2xmlx_xmlLineLength{20}
	@h2xmlx_xmlTabSize{4}			
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}	
   	@h2xmlm_InputPins 				{In1=1;In2=2}												  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111111} 
	@h2xmlx_xmlLineLength{40}
	@h2xmlx_xmlTabSize{1}								
*/
struct param_1{
	

	int a;					/**< @h2xmle_default	{0x4}*/
	short b;   				/**< @h2xmle_default	{0x4}*/
	char c;					/**< @h2xmle_default	{0x4}*/ 					
	long d;					/**< @h2xmle_default	{0x4}*/
};

/** @} */							/* End of Module */			
				