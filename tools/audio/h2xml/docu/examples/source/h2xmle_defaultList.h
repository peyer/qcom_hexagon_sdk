/**************************************************************************//**
* @file
* Example for h2xmle_defaultList
*
******************************************************************************/

#define DEFAULT_VALUE 10

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
	@h2xmlx_expandArray	{false}
*/
struct param_1{
	short x[6];
	/**<		
		@h2xmle_range 				{0x8000..0x7fff}
		@h2xmle_defaultList 		{1,2,3,4,5,6}
	**/	
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmlx_expandArray	{true}
*/
struct param_2{
	short x[6];
	/**<		
		@h2xmle_range 				{0x8000..0x7fff}
		@h2xmle_defaultList 		{1,2,3,4,5,DEFAULT_VALUE}
	**/	
};
	
/** @} */							/* End of Module */						

