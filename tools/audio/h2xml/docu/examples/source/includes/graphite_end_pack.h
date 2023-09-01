/*==============================================================================
  @file graphite_pack_end.h
  @brief This file helps define PACK definition across platfrom

  Copyright (c) 2016 Qualcomm Technologies, Inc.(QTI)
  All rights reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#if defined( __GNUC__ )
  __attribute__((packed))
#elif defined( __arm__ ) || defined( __arm )
#elif defined( _MSC_VER )
  #pragma pack( pop )
#elif defined( __XTENSA__)
  __attribute__((packed))
#elif defined( __H2XML__)
#else
  #error Unsupported compiler.
#endif /* __GNUC__ */


