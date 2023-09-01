/*==============================================================================
  @file graphite_pack_begin.h
  @brief This file helps define PACK definition across platfrom

  Copyright (c) 2016 Qualcomm Technologies, Inc.(QTI)
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#if defined( __GNUC__ )
#elif defined( __arm__ ) || defined( __arm )
  __packed
#elif defined( _MSC_VER )
  #pragma warning( disable:4103 )  /* Another header changing "pack". */
  #pragma pack( push, 1 )
#elif defined( __XTENSA__)
#elif defined( __H2XML__)
#else
  #error Unsupported compiler.
#endif /* __GNUC__ */

