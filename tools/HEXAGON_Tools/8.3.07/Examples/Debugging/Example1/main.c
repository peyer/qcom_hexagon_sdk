/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

// needed for printf
#include <stdio.h>

// this structure will have 4 byte alignment
// because it's largest value is int
struct unpacked_struct
{
	char a;
	int i1;
	int i2;
};

// initialize the unpacked struct
struct unpacked_struct my_unpacked_struct = { 'a', 0x11, 0x22 };

// this structure will have 1 byte alignment
// because it is packed
struct packed_struct
{
	char a;
	int i1;
	int i2;

} __attribute__ ((__packed__));

// initialize the packed struct
struct packed_struct my_packed_struct = { 'b', 0x33, 0x44 };

int main(int argc, char **argv)
{
	printf("\nVar unpacked i2 = 0x%x\n", my_unpacked_struct.i2);

	printf("\nVar packed i2 = 0x%x\n", my_packed_struct.i2);

	return 0;
}
