/**************************************************************************//**
* @file
* Example for h2xmle_increment
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
	@h2xmle_increment	{1}									<--! default for this parameter -->
*/
struct param_1{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_increment	{2} */
	int c;   			/**<		@h2xmle_increment	{10} 	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
	@h2xmle_increment	{10}									<--! default for this parameter -->
*/
struct param_2{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_increment	{1} */
	int c;   			/**<		@h2xmle_increment	{2} 	*/
};

/** @h2xmlp_parameter 	{Parameter3,0x33333399} 
																<--! no default specified -->
*/
struct param_3{
	unsigned short a;	/**<									*/	
	int b;   			/**<		@h2xmle_increment	{10} */
	int c;   			/**<		@h2xmle_increment	{1} 	*/
	int d;   			/**<		@h2xmle_increment	{100} 	*/

};

/** @} */							/* End of Module */						

