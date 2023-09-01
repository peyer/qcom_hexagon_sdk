/**************************************************************************//**
* @file
* Example for h2xmle_rangeList
******************************************************************************/

#define value1	21
/*------------------------------------------------------------------------*//**
 	@h2xmlm_module 					{MODULE_1,0x11111111  }			  
   	@{
----------------------------------------------------------------------------*/



/** @h2xmlp_parameter 	{Parameter2,0x22222299} 
*/
struct param_2{
	char a;				
	/**<		@h2xmle_rangeList {	label_1 = 1  ;
									label_2 = 2  ;
									label_3 = 3  ;
									label_4 = 4  ;
									label_5 = 5  ;
									label_6 = 6  ;
									label_7 = 7  ;
									label_8 = 8  ;
									label_9 = 9  ;
									label_10= 10 ;
									label_11= 11 ;
									label_12= 12 ;
									label_13= 13 ;
									label_14= 14 ;
									label_15= 15 ;
									label_16= 16 ;
									label_17= 17 ;
									label_18= 18 ;
									label_19= 19 ;
									label_20= 20 ;
									label_21= value1 
									}	
				@h2xmle_default {value1} 
	*/
	long b;   			/**<		@h2xmle_rangeList {a=-1; b=0x20; c=0x0} */
};

/** @} */							/* End of Module */						

