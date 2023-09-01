/**************************************************************************//**
* @file
* * Example for h2xmlx_expandArray
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }												  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 			{Parameter1,0x11111111} 
	@h2xmlx_expandArrayOffset	{1}													
*/
struct param_1{
	int a[10][3];					
	short b[20];   				
	char c[30];					/**< @h2xmlx_expandArray{false} */ 				
	long d[40];						
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
				