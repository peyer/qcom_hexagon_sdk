/**************************************************************************//**
* @file
* Example for h2xmle_defaultDependency with default lists
*
******************************************************************************/

#define DEFAULT_VALUE 10

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	char a[6];				
	/**<		
		@h2xmle_range 				{0x80..0x7f} 
		@h2xmle_default 			{10}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, Samplerate="8000", defaultList="{1,2,3,4,5,DEFAULT_VALUE}"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", defaultList="{10,20,30,40,50,60}"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", defaultList="{11,22,33,44,55,66}"}
		
	*/
	long b;   			
	/**<		
		@h2xmle_range {-5..20} 
	*/

};

/** @} */							/* End of Module */						

