/**************************************************************************//**
* @file
* Example for h2xmle_defaultDependency
*
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	char a;				
	/**<		
		@h2xmle_range 				{0x80..0x7f} 
		@h2xmle_default 			{10}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, Samplerate="8000", default="2"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", default="3"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", default="4"}
		
	*/
	long b;   			
	/**<		
		@h2xmle_range {-5..20} 
	*/
	char c;				
	/**<		
		@h2xmle_range 				{0x80..0x7f} 
		@h2xmle_default 			{10}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, default="2"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", Samplerate="16000", default="3"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", default="40"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", default="3"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", default="3"}

		
	*/
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		
		@h2xmle_range 				{0x80..0x7f} 
		@h2xmle_default 			{10}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, Samplerate="8000", default="2"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", default="3"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", default="4"}	
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", VoiceBandWidth="WB", Samplerate="48000",  default="4"}	*/

	long b;   			
	/**<		
		@h2xmle_range {-5..20} 
	*/
};

/** @} */							/* End of Module */						

