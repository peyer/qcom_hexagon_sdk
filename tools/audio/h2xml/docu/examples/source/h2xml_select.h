/**************************************************************************//**
* @file
* Example for h2xml_select 
******************************************************************************/

typedef struct {
	int exa;			/**< @h2xmle_default	{0x1}*/
	int exb;			/**< @h2xmle_default	{0x2}*/
	int exc;			/**< @h2xmle_default	{0x3}*/
} sStruct1_t;

typedef struct {
	sStruct1_t s1;	
	char exy;			/**< @h2xmle_default	{0x4}*/
	short exz;			/**< @h2xmle_default	{0x5}*/
}sStruct2_t;


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter {Parameter1,0x11111111} */
struct param_1{
	short 	a1;		/**< @h2xmle_default		{0x88} */
	int 	b1;   	/**< @h2xmle_default		{0x99} */
	sStruct2_t c1;	
};

/**
	@h2xml_select 	{param_1::c1::s1::exa}
	@h2xmle_default	{0x11}
	@h2xml_select 	{sStruct2_t::exy}
	@h2xmle_default	{0x44}
*/


/** @} */							/* End of Module */						

