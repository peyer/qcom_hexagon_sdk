#/* ======================================================================= */
#/*  QUALCOMM TECHNOLOGIES, INC.                                            */
#/*                                                                         */
#/*  HVX Intrinsics Examples with Boundary Conditions                       */
#/*                                                                         */
#/* ----------------------------------------------------------------------- */
#/*          Copyright (c) 2016 QUALCOMM TECHNOLOGIES Incorporated.         */
#/*                           All Rights Reserved.                          */
#/*                  QUALCOMM Confidential and Proprietary                  */
#/* ======================================================================= */

CURR_DIR = $(shell pwd)
DEVICE_STANDALONE_TEST_TOP_DIR = ${CURR_DIR}/../..
TESTNAME = $(shell basename ${CURR_DIR})
TESTIMAGE1 = $(CURR_DIR)/../../Images/img11_p010_ubwc_hana.bin
TESTIMAGE2 = $(CURR_DIR)/../../Images/img11_p010.bin

FEATURES ?= hexagon_dma-hvx_128
include $(DEVICE_STANDALONE_TEST_TOP_DIR)/apps/Makefile.common

TEST_ARGS_128=1024 1024 1024 4 1 "/data/img11_p010_ubwc_hana.bin" "/data/img11_p010.bin"
TEST_ARGS_64=1024 1024 1024 4 1 "/data/img11_p010_ubwc_hana.bin" "/data/img11_p010.bin"

include $(DEVICE_STANDALONE_TEST_TOP_DIR)/apps/Makefile.rules

