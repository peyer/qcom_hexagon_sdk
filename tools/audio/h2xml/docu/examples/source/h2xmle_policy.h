/**************************************************************************//**
* @file
* Example for h2xmle_policy
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
	@h2xmle_policy	{basic}									<--! default for this parameter -->
*/
struct param_1{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_policy	{advanced} */
	int c;   			/**<		@h2xmle_policy	{internal} 	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmle_policy	{internal}									<--! default for this parameter -->
*/
struct param_2{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_policy	{basic} */
	int c;   			/**<		@h2xmle_policy	{advanced} 	*/
};

/** @h2xmlp_parameter 	{Parameter3,0x33333399} 
																<--! no default specified -->
*/
struct param_3{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_policy	{internal} */
	int c;   			/**<		@h2xmle_policy	{basic} 	*/
};

/** @} */							/* End of Module */						

