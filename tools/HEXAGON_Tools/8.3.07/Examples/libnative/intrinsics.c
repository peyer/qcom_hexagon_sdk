
#include <stdio.h>
#include <hexagon_protos.h>

int main(int argc, char **argv)
{
	unsigned int a = 2;
	unsigned int b = 3;
	unsigned long long c = 0;

	c = Q6_R_add_RR(a, b);
	c = Q6_P_mpy_RR(c, b);

	printf("a = %d\n", a);
	printf("b = %d\n", b);
	printf("(a + b)*b = %llu\n", c);

	return 0;
}
