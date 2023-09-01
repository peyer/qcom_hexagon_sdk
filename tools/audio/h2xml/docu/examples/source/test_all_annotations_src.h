/**************************************************************************//**
* @file
* uses all available annotations
* Note: Values or combination of values might not make sense!
******************************************************************************/


#define DSP_TYPE						WDSP 
#define AVS_BUILD_MAJOR_VERSION   		2
#define AVS_BUILD_MINOR_VERSION   		8
#define AVS_BUILD_BRANCH_VERSION  		16
#define AVS_BUILD_SUBBRANCH_VERSION  	32

#define MODULE_NEW 0x44444444
#define MODULE_NEW2 0x56785678

/**
	@h2xml_setVersion				{DSP_TYPE;AVS_BUILD_MAJOR_VERSION;AVS_BUILD_MINOR_VERSION;AVS_BUILD_BRANCH_VERSION;AVS_BUILD_SUBBRANCH_VERSION}
*/


#define 	PID1	0x12345678
#define 	PID2  	0x87654321
#define 	PARAM1_VERSION	1


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }
	@h2xmlm_pidFwk					{PID1}		
 	@h2xmlm_PinType 				{static}									
	@h2xmlm_InputPinsIdSize 		{2}								
   	@h2xmlm_InputPins 				{In1=1;In2=2}
 	@h2xmlm_OutputPinsIdSize 		{4}									
 	@h2xmlm_OutputPins 				{PIN1=0;PIN1=7;PIN2=15;PIN5=0x123}	
 	@h2xmlm_replacedBy				{MODULE_NEW}			
 	@h2xmlm_ToolPolicy 				{Calibration; RTC; RTM; NO_SUPPORT}	
 	@h2xmlm_deprecated				
 	@h2xmlm_Description 			{This is the description for Module1\n
 									description1 description1 description1 description1
 									description1 description1 description1 description1
 									description1 description1 description1 description1
 									description1 description1 description1 description1
 									description1 description1 description1 description1
 									}		
   	@{
----------------------------------------------------------------------------*/

/** 
	@h2xmlp_parameter {Parameter1,0x11111199} 
		@h2xmlp_description	{
			Description of Parameter 1. Description continued.\n
			Description continued.\n Description continued.\n
			Description continued.\n}
		@h2xmlp_toolPolicy	{Calibration; RTC; RTM; NO_SUPPORT}
		@h2xmlp_version		{PARAM1_VERSION}
*/
struct param_1{
	short a;
	/**<
		@h2xmle_name 		{NameForA}				<!-- default variable name is overridden -->
		@h2xmle_default		{-1}
		@h2xmle_visibility	{hide}
		@h2xmle_readonly	{false}	
		@h2xmle_rangeList 	{a=-1; b=0x20} 		
		@h2xmle_range 		{-10..20}
		@h2xmle_policy		{advanced}
		@h2xmle_displayType	{slider}
		@h2xmle_description {This is element a. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
		@h2xmle_dataFormat	{q18}
		@h2xmle_isVersion	{true}
	*/
	int b;   
    /**<
 		@h2xmle_default		{0x3}
		@h2xmle_visibility	{show}
		@h2xmle_readonly	{true}
		@h2xmle_rangeList {	enable1=4; 
							enable2=1;
							enable3=3;
							enable4=55
		}	
		@h2xmle_range 		{0..20}
		@h2xmle_policy		{internal}
		@h2xmle_displayType	{dropDown}
		@h2xmle_description {This is element b. Further description, 
			Further description, Further description, Further description, 
			Further description, Further description} 
		@h2xmle_dataFormat	{Q8}
	*/	
	unsigned char c;
	/**<
		@h2xmle_policy		{basic}
		@h2xmle_range 		{0..64}
		@h2xmle_increment	{2}
		@h2xmle_displayType	{dump}
	*/
	
	long	d;
	/**<
		@h2xmle_displayType	{checkBox} 
	*/
	
	unsigned char	e[32];
	/**< 
		@h2xmle_defaultFile {defaultFile2.bin}					<!--Parameters hex string will reflect binary file -->
		@h2xmle_default 	{0}									<!--default values will be replaced by values from file -->
	*/
	
	/////////////////////////////////////////////////////////
	// Example for bitfields
	/////////////////////////////////////////////////////////
	unsigned int f;   			
	/**<		
		@h2xmle_bitfield			{0x00000001}
			@h2xmle_bitName			{Bit_0}
			@h2xmle_description		{Description of Bit[0]}
			@h2xmle_default			{1}
		@h2xmle_bitfieldEnd
		
		@h2xmle_bitfield			{0x00000006}
			@h2xmle_bitName			{Bit_2_1}
			@h2xmle_description		{Description of Bit[2:1]}
			@h2xmle_default			{2}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield			{0x000000f0}
			@h2xmle_bitName			{Bit_7_4}
			@h2xmle_description		{Description of Bit[7:4]}
			@h2xmle_default			{7}
			@h2xmle_rangeList		{x0=0;x1=1;x2=2;x3=3;x4=7}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield			{0xff000000}
			@h2xmle_bitName			{Bit_31_24}
			@h2xmle_description		{Description of Bit[31:24}
			@h2xmle_default			{4}
			@h2xmle_range			{1..10}
		@h2xmle_bitfieldEnd
	 */
	
	long x;
	
};

/** @} */							/* End of Module */						


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_2,0x22222222  }
	@h2xmlm_pidFwk					{PID2}		  
	@h2xmlm_PinType 				{dynamic}
   	@h2xmlm_InputPins 				{In1=2;In2=3}							
   	@h2xmlm_OutputPins 				{Out1=4;Out2=5}	
 	@h2xmlm_replacedBy				{MODULE_NEW2}
 	@h2xmlm_ToolPolicy 				{Calibration; RTM}		
 	@h2xmlm_deprecated				{false}		
	@h2xmlm_Description 			{This is the description for Module2\n
 									description2 description2 description2 description2
 									description2 description2 description2 description2
 									description2 description2 description2 description2
 									description2 description2 description2 description2
 									description2 description2 description2 description2
 									}	  	
   	@{								
----------------------------------------------------------------------------*/

/** 
	@h2xmlp_parameter {Parameter2,0x22222299} 
	@h2xmlp_description	{
		Description of Parameter 2. Description continued.\n
		Description continued.\n Description continued.\n
		Description continued.\n}
	@h2xmlp_toolPolicy	{Calibration; NO_SUPPORT}
	@h2xmlp_version		{2}
	@h2xmle_visibility	{show}
	@h2xmle_readonly	{true}
	@h2xmle_policy		{advanced}
*/
struct param_2{
	int a;
	/**<
		@h2xmle_default		{0x33}
		@h2xmle_policy		{basic}
		@h2xmle_increment	{10}

	*/
	short b;   
    /**<
 		@h2xmle_default		{0x44}
		@h2xmle_range 		{0..0x55}

	*/	
};

/** @} */							/* End of Module */		

long function1(int a, int b, float c);

int main()
{
	int a=0;
	int b=3*4;
	int x= 3*4+5/6|7&8<<9>>10*(8+9);
	
	long y=function1(1,2,3);
	
	for (a=0;a<100;a++) {
		long c=0;
	}
	float ff=a*b+a/x & a && b | a ||b >>c >c <a <<a ;
	
	switch (a) {
		case 1:
			b=2;
		case 2: 
			b=3;
		default:
			b=4;
	};
	
	while (b) {
		b--;
		a++;
		a*=2;
		b*=3;
		c/=4;
		x&=x;
		x|=x;
		x+=x;
		x-=x;
		break;
	}
	if (x>b) {
		x++;
	} else {
		x--;
	}
	
	
	return 0;
}				

long function1(int a, int b, float c)
{
	return 1;
}
