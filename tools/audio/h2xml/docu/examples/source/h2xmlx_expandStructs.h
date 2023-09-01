/**************************************************************************//**
* @file
* Example for h2xmlx_expandStructs
******************************************************************************/


typedef struct {
	int s1a;					/**< @h2xmle_default {0x12345678} @h2xmle_range{0x10000000..0x20000000} */
	int s1b;					/**< @h2xmle_default {0x11223344} */
	int s1c;					/**< @h2xmle_default {0x55667788} */
} sStruct1_t;

typedef struct {
	int s2a;					/**< @h2xmle_default {0x77654321} */
	sStruct1_t s2b;
	sStruct1_t s2c;
} sStruct2_t;


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
 	@h2xmlx_expandStructs			{false}										  									  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 			{Parameter1,0x11111111} */
struct param_1{		
	int b; /**< @h2xmle_default {0x55556666} */
	sStruct1_t p1a;					
	sStruct2_t p1b;					
		
};

/** @h2xmlp_parameter 			{Parameter2,0x22222222} 
 	@h2xmlx_expandStructs		{true}										  									  
*/
struct param_2{
	sStruct1_t p1a;					
	sStruct2_t p1b;	
};

/** @h2xmlp_parameter 			{Parameter3,0x33333333} */
struct param_3{
	sStruct1_t p1a;					
	sStruct2_t p1b;	
};

/** @h2xmlp_parameter 			{Parameter4,0x44444444} 
 	@h2xmlx_expandStructs		{true}										  									  
*/
struct param_4{
	sStruct1_t p1a;					
	sStruct2_t p1b;	
};

/** @} */							/* End of Module */			

			