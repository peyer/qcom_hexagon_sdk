/**************************************************************************//**
* @file
* Example for h2xmle_elementType
* overrides type definition from c-code
*******************************************************************************/

typedef unsigned int uint32;
typedef uint32  newUint32_t;


typedef struct {
	newUint32_t s1a;
	unsigned char s1b;					
}sStruct1_t;



/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** 
	@h2xmlp_parameter 			{Parameter1,0x11111199} 
	@h2xmlx_expandTypeDefs		{false}
*/
struct param_1{
	short 				a;			/**<  @h2xmle_elementType {type1} 	*/
	unsigned short 		b;			/**<  @h2xmle_elementType {type2} 	*/
	uint32				c;			/**<  @h2xmle_elementType {type3} 	*/
	newUint32_t			d;			/**<  @h2xmle_elementType {type4} 	*/
	sStruct1_t 			e;			/**<  @h2xmle_elementType {type5} 	*/						
};

/** 
	@h2xmlp_parameter 			{Parameter2,0x22222299} 
	@h2xmlx_expandTypeDefs		{true}
*/
struct param_2{
	short 				a;			/**<  @h2xmle_elementType {type1} 	*/
	unsigned short 		b;			/**<  @h2xmle_elementType {type2} 	*/
	uint32				c;			/**<  @h2xmle_elementType {type3} 	*/
	newUint32_t			d;			/**<  @h2xmle_elementType {type4} 	*/
	sStruct1_t 			e;			/**<  @h2xmle_elementType {type5} 	*/					
};

/** @} */							/* End of Module */						

