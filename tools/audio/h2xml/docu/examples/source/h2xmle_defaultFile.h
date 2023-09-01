/**************************************************************************//**
* @file
* Example for h2xmle_defaultFile
******************************************************************************/

typedef struct {
	long a;
	short b;
	short c;
} sStruct1_t;

typedef struct {
	long x[8];
} sStruct2_t;

/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/

/** @h2xmlp_parameter 	{Parameter1,0x11111199} 
*/
struct param_1{
	unsigned char a[32];	/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
};

/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	unsigned char a[32];	/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	sStruct1_t   s1[4];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	sStruct2_t	 s2;		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/		
	char b;					/**<		@h2xmle_default {0x44} 		*/
	long c;   				/**<		@h2xmle_default {0x17654321} 	*/
	sStruct1_t   s3[0];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
};

/** @h2xmlp_parameter 		{Parameter3,0x33333399} 
	@h2xmlx_ExpandStructs	{false}
*/
struct param_3{
	unsigned char a[32];	/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	sStruct1_t   s1[4];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	sStruct2_t	 s2;		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	char b;					/**<		@h2xmle_default {0x44} 		*/
	long c;   				/**<		@h2xmle_default {0x17654321} 	*/
	sStruct1_t   s3[0];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
};

/** @h2xmlp_parameter 		{Parameter4,0x44444499} 
	@h2xmlx_ExpandStructs	{false}
	@h2xmlx_ExpandArray		{false}
*/
struct param_4{
	unsigned char a[32];	/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
	sStruct1_t   s1[4];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/
	sStruct2_t	 s2;		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/		
	char b;					/**<		@h2xmle_default {0x44} 		*/
	long c;   				/**<		@h2xmle_default {0x17654321} 	*/
	sStruct1_t   s3[0];		/**<		@h2xmle_defaultFile {defaultFile2.bin} 		*/	
};



/** @} */							/* End of Module */						

