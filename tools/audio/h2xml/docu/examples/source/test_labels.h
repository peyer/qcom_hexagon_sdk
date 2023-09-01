/**************************************************************************//**
* @file
* Example for various label definitions using quotes 
******************************************************************************/

#define MODULE_1_ID	0x11111111
#define value1		1
/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{"MODULE_1_ID",MODULE_1_ID  }			
 	<!-- use quotes, otherwise MODULE_1_ID will be recognized as number -->  
   	@{
----------------------------------------------------------------------------*/



/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		@h2xmle_rangeList {	"value1" = value1  ;	<!-- use quotes, otherwise value1 will be recognized as number -->
									label_2 = 2  ;			<!-- label_2 is a regular c identifier and has no other definition. No quotes needed -->
									"3"= 3;					<!-- make sure "3" is evaluated as label -->
									"~77 	#6" = 4			<!-- not a regular c type label. Use quotes -->
									}	
				@h2xmle_default {value1}
	*/
};

/** @} */							/* End of Module */						

