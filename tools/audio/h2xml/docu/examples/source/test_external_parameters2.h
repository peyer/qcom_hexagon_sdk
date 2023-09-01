/**************************************************************************//**
* @file
* Example for globally defined parameters that are used in several modules 
* @parameter definition not in global struct but within the module
******************************************************************************/

typedef struct {
	int exa;		/**< @h2xmle_default		{0x1}*/
	int exb;		/**< @h2xmle_default		{0x2}*/
	int exc;		/**< @h2xmle_default		{0x3}*/
} extParam1;

struct extParam2{
	int exx;		/**< @h2xmle_default		{0x4}*/
	char exy;		/**< @h2xmle_default		{0x5}*/
	short exz;		/**< @h2xmle_default		{0x6}*/
};


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
   	@h2xmlm_InputPins 				{In1=1;In2=2}							
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}					  
   	@{
----------------------------------------------------------------------------*/

/**
	@h2xml_Select					{extParam1}
 	@h2xmlp_parameter 				{ExternalParameter1, 0x12345678} 
    @h2xmlm_InsertParameter	
	@h2xmlp_description				{Description External Parameter1 Module 1}
	@h2xml_Select					{extParam1::exa}	
	@h2xmle_default					{11}
	@h2xml_Select					{extParam1::exb}	
	@h2xmle_default					{22}
*/

/**
	@h2xml_Select					{extParam2}
	@h2xmlp_parameter 				{ExternalParameter2, 0x87654321}
    @h2xmlm_InsertParameter	
	@h2xmlp_description				{Description External Parameter2 Module 1}
	@h2xml_Select					{extParam2::exx}	
	@h2xmle_default					{0x111}
	@h2xml_Select					{extParam2::exy}	
	@h2xmle_default					{0x32}
*/

/** @h2xmlp_parameter {Parameter3,0x11111199} */
struct param_1{
	short 	a1;		/**< @h2xmle_default		{0x33} */
	int 	b1;   	/**< @h2xmle_default		{0x44} */	
};

/** @} */							/* End of Module */						


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_2,0x22222222  }
   	@h2xmlm_InputPins 				{In1=2;In2=3}							
   	@h2xmlm_OutputPins 				{Out1=4;Out2=5}					  
   	@{								
----------------------------------------------------------------------------*/

/**
	@h2xmlp_insertParameter			{extParam1}
	@h2xmlp_parameter 				{ExternalParameter1, 0x02345678} 
	@h2xmlp_description				{Description External Parameter1 Module 2}
	@h2xml_Select					{extParam1::exa}	
	@h2xmle_default					{0x555}
	@h2xml_Select					{extParam1::exb}	
	@h2xmle_default					{0x666}
*/

/**
	@h2xml_Select					{extParam2}
	@h2xmlp_parameter 				{ExternalParameter2, 0x07654321}
    @h2xmlm_InsertParameter	
	@h2xmlp_description				{Description External Parameter2 Module 1}
	@h2xml_Select					{extParam2::exx}	
	@h2xmle_default					{0x1111}
	@h2xml_Select					{extParam2::exy}	
	@h2xmle_default					{0x23}
*/

/** @h2xmlp_parameter {Parameter4,0x22222299} */
struct param_1{
	int 	a2;		/**< @h2xmle_default		{0x55} */
	short 	b2;   	/**< @h2xmle_default		{0x66} */	
};

/** @} */								/* End of Module */