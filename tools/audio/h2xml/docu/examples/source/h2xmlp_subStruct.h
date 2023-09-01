/**************************************************************************//**
* @file
* Example for h2xmlp_subStruct
******************************************************************************/

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_subStruct */
typedef struct {
	unsigned short a; 	/**<@h2xmle_default {0x1122} 		*/
	unsigned int b;   	/**<@h2xmle_default {0x88776655} 	*/
} sStruct1_t;

/** @h2xmlp_subStruct */
struct struct2{
	unsigned short a; 	/**<@h2xmle_default {0x1122} 		*/
	unsigned int b;   	/**<@h2xmle_default {0x88776655} 	*/
} ;

/** 
	@h2xmlp_parameter 		{Parameter2,0x22222299} 
	@h2xmlx_expandStructs	{false}
*/
struct param1{
	char a;				/**<@h2xmle_default {0x55} 		 */
	long b;   			/**<@h2xmle_default {0x11111111} */
	sStruct1_t 	sub1;	
	struct2 	sub2[4];
};

/** @} */							/* End of Module */						

