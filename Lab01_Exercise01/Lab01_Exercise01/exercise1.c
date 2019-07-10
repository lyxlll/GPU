#include <stdio.h>
#include "random.h"
#define NUM_VALUES 250
signed int value[NUM_VALUES];

int main() {
	unsigned int sum = 0;
	unsigned int i = 0;
	init_random();
	for (i ; i < NUM_VALUES; i++)
	{
		value[i] = (int)random_ushort();
		printf("%d : %d\n", i, value[i]);
		sum += value[i];
	}
	int average = sum / NUM_VALUES;
	int max = INT_MIN;
	int min = INT_MAX;
	for ( i = 0; i < NUM_VALUES; i++)
	{
		max = (max > (value[i] - average)) ? max : (value[i] - average);
		min = (min > (value[i] - average)) ? (value[i] - average) : min;
	}
	printf("Sum: %d, Average: %d\n", sum, average);
	printf("Min: %d, Max: %d\n", min, max);
	system("pause");
	return 0;
}