/*!************************************************************************//*!
* @file
* comments before or after identifier or via reference
* using /*! and //!
******************************************************************************/

/*------------------------------------------------------------------------*//*!
 	@h2xmlm_module 					{MODULE_1,0x11111111  }					<!-- Module annotations MUST always be before module compound -->
   	@h2xmlm_OutputPins 				{Out1=3;Out2=4}	
   	@h2xmlm_InputPins 				{In1=1;In2=2}												  
   	@{
----------------------------------------------------------------------------*/

/*! @h2xmlp_parameter 	{Parameter1,0x11111111} 							<!-- refers to  param_1 -->
*/
struct param_1{
	
	/*!
		@h2xmle_default		{0x11}											<!-- refers to  a -->
	*/
	int a;
	short b;   
    /*!<
 		@h2xmle_default		{0x22}											<!-- refers to  b -->
	*/	
	char c;					/*!<@h2xmle_default		{0x33} 					<!-- refers to  c --> */
	long d;					 
	//! @h2xmle_default		{0x55} 											<!-- refers to  e -->
	long e;	
	long f;				 	//!<@h2xmle_default		{0x66} 					<!-- refers to  f -->
};
/*!< 																		<!-- refers to param_1 -->
	@h2xmlp_description		{Description Parameter 1}
*/


/*!
	@h2xml_select			{param_1::d}
	@h2xmle_default			{0x44444444}									<!-- refers to param_1::d --> 
*/

struct param_2{
	
	int a;
	/*!<
		@h2xmle_default		{0x11}											<!-- refers to  a -->
	*/
    /*!
 		@h2xmle_default		{0x22}											<!-- refers to  b -->
	*/	
	short b;   
	
	char c;							
	long d;
	//! @h2xmle_default		{0x55}											<!-- refers to  e -->
	char e;							
	long f;
	//!< @h2xmle_default	{0x66}											<!-- refers to  r -->

};
/*!< 	@h2xmlp_parameter 	{Parameter2,0x22222222} 						<!-- refers to  param_2 -->
	 	@h2xmlp_description	{Description Parameter 2}
*/

struct param_3{
	
	int a;
	/*!<
		@h2xmle_default		{0x11}											<!-- refers to  a -->
	*/
    /*!
 		@h2xmle_default		{0x22}											<!-- refers to  b -->
	*/	
	short b;   
	char c;							
	long d;
};

/*! @} */							/* End of Module */			
				

/*!
	@h2xml_select			{param_2::d}
	@h2xmle_default			{0x44444444}									<!-- refers to  param_2::d -->
	@h2xml_select			{param_2::c}
	@h2xmle_default			{0x33}											<!-- refers to  param_2::c -->
*/

/*!
	@h2xml_select			{param_3}
	@h2xmlp_parameter 		{Parameter3,0x33333333} 						<!-- refers to  param_3 -->
	@h2xmlp_description		{Description Parameter 3}
	@h2xml_select			{param_3::d}
	@h2xmle_default			{0x44444444}									<!-- refers to  param_3::d -->
	@h2xml_select			{param_3::c}
	@h2xmle_default			{0x33}											<!-- refers to  param_3::c -->
*/