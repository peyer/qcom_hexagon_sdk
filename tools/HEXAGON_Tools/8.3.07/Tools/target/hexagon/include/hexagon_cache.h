/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef HEXAGON_CACHE_H
#define HEXAGON_CACHE_H
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void hexagon_buffer_clean (const uint8_t *addr, size_t nbytes);
void hexagon_buffer_cleaninv (const uint8_t *addr, size_t nbytes);
void hexagon_buffer_inv (const uint8_t *addr, size_t nbytes);

/*** backward compatibility aliases  ***/
void q6_buffer_clean (const uint8_t *addr, size_t nbytes);
void q6_buffer_cleaninv (const uint8_t *addr, size_t nbytes);
void q6_buffer_inv (const uint8_t *addr, size_t nbytes);

#ifdef __cplusplus
};
#endif

#endif



