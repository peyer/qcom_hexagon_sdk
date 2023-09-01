/**************************************************************************//**
* @file
* Example for h2xmle_isVersion
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter3,0x11111111} 
*/
struct param_1{
	unsigned short a;	/**<									*/	
	int b;   			/**<									*/
	int version;   		/**<		@h2xmle_isVersion	{true} 	*/
};

/** @h2xmlp_parameter 	{Parameter1,0x22222222} 
	@h2xmle_isVersion	{true}									<--! default for this annotation -->
*/
struct param_2{
	unsigned short a;	/**<		@h2xmle_isVersion	{false}	*/	
	int b;   			/**<		@h2xmle_isVersion	{false} */
	int version;   		
};

/** @h2xmlp_parameter 	{Parameter2,0x33333333} 
	@h2xmle_isVersion	{false}									<--! default for this annotation -->
*/
struct param_3{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_isVersion	{false} */
	int version;   		/**<		@h2xmle_isVersion	{true} 	*/
};



/** @} */							/* End of Module */						

