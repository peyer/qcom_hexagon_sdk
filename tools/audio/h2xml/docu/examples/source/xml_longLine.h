/**************************************************************************//**
* @file
* Check that a very long attribute value is split correctly
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
 	 @h2xmlx_xmlLineLength{20}	
 	 @h2xmlx_xmlAttributeLength{60}										  									  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 			{Parameter1,0x11111111} 
*/
struct param_1{		
	char a;					
	/**< @h2xmle_description	{
	This is a very long description blahblahblah
	blahblahblah blahblahblah blahblahblah
	blahblahblah blahblahblah blahblahblah
	blahblahblah blahblahblah blahblahblah
	blahblahblah blahblahblah blahblahblah	
	blahblahblah blahblahblah blahblahblah	
	blahblahblah blahblahblah blahblahblah	
	blahblahblah blahblahblah blahblahblah	
	} 
	@h2xmle_readOnly		{true}
	
	*/ 				
};

/** @h2xmlp_parameter 			{Parameter1,0x11111111} 
	@h2xmlx_expandArray			{false}
	@h2xmlx_expandArrayOffset	{0}															
*/
struct param_2{
	int a[10];					
	short b[20];   					/**< @h2xmlx_expandArray{true} @h2xmlx_expandArrayOffset{5}*/
	char c[30][2];					/**< @h2xmlx_expandArray{true} @h2xmlx_expandArrayOffset{3}*/ 					
	long d[40];					
};


/** @} */							/* End of Module */			
				