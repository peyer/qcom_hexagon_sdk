/**************************************************************************//**
* @file
* Example for h2xmle_defaultDependency with default files
*
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	char a[32];				
	/**<		
		@h2xmle_range 				{0x80..0x7f} 
		@h2xmle_default 			{10}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, Samplerate="8000", defaultFile="defaultFile2.bin"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", defaultFile="defaultFile2.bin"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", defaultFile="defaultFile2.bin"}
		
	*/
	long b;   			
	/**<		
		@h2xmle_range {-5..20} 
	*/
	unsigned short c[0];				
	/**<		
		@h2xmle_range 				{0x0000..0xffff} 
		@h2xmle_default 			{100}
		@h2xmle_defaultDependency 	{direction="TX", device=Handset, AudioVoice=Audio, Samplerate="8000", defaultFile="defaultFile2.bin"}
		@h2xmle_defaultDependency 	{direction="RX", device="Handset", AudioVoice="Audio", Samplerate="16000", defaultFile="defaultFile2.bin"}
		@h2xmle_defaultDependency 	{direction="TX", device="Handset", AudioVoice="Audio", Samplerate="48000", defaultFile="defaultFile2.bin"}
		
	*/
};

/** @} */							/* End of Module */						

