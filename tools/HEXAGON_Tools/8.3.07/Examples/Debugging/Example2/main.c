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

// this structure will have 1 byte alignment
// because it is packed
struct packed_struct
{
	int i;
	char c;
	struct unpacked_struct s;

} __attribute__ ((__packed__));


// initialize the packed struct
struct packed_struct my_packed_struct = { 0x33, 'a', 'b', 0x55, 0x66 };

// create a pointer to the packed structure
struct packed_struct *ptr_to_packed = &my_packed_struct;

// create a pointer to the unpacked structure
struct unpacked_struct *ptr_to_unpacked = &my_packed_struct.s;

int main(int argc, char **argv)
{
	printf("\nVariable i2 = 0x%x\n", ptr_to_packed->s.i2);

	printf("\nVariable i2 = 0x%x\n", ptr_to_unpacked->i2);

	return 0;
}
