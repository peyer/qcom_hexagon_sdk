/**************************************************************************//**
* @file
* Example for h2xmle_bitfield
******************************************************************************/


/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }		
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned int a;   			
	/**<		
		@h2xmle_bitfield		{0x00000001}
		@h2xmle_bitName			{Bit_0}
		@h2xmle_bitfieldEnd
		
		@h2xmle_bitfield		{0x00000006}
		@h2xmle_bitName			{Bit_2_1}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield		{0x000000f0}
		@h2xmle_bitName			{Bit_7_4}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield		{0xff000000}
		@h2xmle_bitName			{Bit_31_24}
		@h2xmle_bitfieldEnd
	 */
	 
	 unsigned int b;   			
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
			@h2xmle_description		{Description of Bit[31:24]}
			@h2xmle_default			{4}
			@h2xmle_range			{1..10}
		@h2xmle_bitfieldEnd
	 */
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	unsigned int a;   			
	/**<		
		@h2xmle_bitfield		{0x00000001}
		@h2xmle_bitName			{Bit_0}
		@h2xmle_bitfieldEnd
		
		@h2xmle_bitfield		{0x00000006}
		@h2xmle_bitName			{Bit_2_1}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield		{0x000000f0}
		@h2xmle_bitName			{Bit_7_4}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield		{0xff000000}
		@h2xmle_bitName			{Bit_31_24}
		@h2xmle_bitfieldEnd
	 */
	 
	 unsigned int b;   			
	/**<		
		@h2xmle_bitfield			{0x00000001}
			@h2xmle_bitName			{Bit_0}
			@h2xmle_description		{Description of Bit[0]}
			@h2xmle_default			{1}
			@h2xmle_visibility		{show}
		@h2xmle_bitfieldEnd
		
		@h2xmle_bitfield			{0x00000006}
			@h2xmle_bitName			{Bit_2_1}
			@h2xmle_description		{Description of Bit[2:1]}
			@h2xmle_default			{2}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield			{0x000000f0}
			@h2xmle_bitName			{Bit_7_4}
			@h2xmle_description		{Description of Bit[7:4]}
			@h2xmle_default			{3}
			@h2xmle_rangeList		{x0=0;x1=1;x2=2;x3=3}
		@h2xmle_bitfieldEnd

		@h2xmle_bitfield			{0xff000000}
			@h2xmle_bitName			{Bit_31_24}
			@h2xmle_description		{Description of Bit[31:24]}
			@h2xmle_default			{4}
			@h2xmle_range			{1..10}
			@h2xmle_increment		{3}
		@h2xmle_bitfieldEnd
	 */
};

/** @} */							/* End of Module */						

