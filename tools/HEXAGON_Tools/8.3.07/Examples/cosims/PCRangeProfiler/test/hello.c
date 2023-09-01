#include <stdio.h>

void StartIteration() {}
void EndIteration() {}

int main()
{
	unsigned int i,j;

	for(i = 0; i < 10; i++)
	{
		StartIteration();
		for(j = 0; j <= (10 - i); j++)
			printf("Hello world!\n");
		EndIteration();
	}

	printf("Goodbye!\n");
}
