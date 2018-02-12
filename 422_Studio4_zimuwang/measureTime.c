#include <stdio.h>
#include <time.h>

int main()
{
	struct timespec first;
	struct timespec second;
	long diff = 0;
	
	clock_gettime(CLOCK_MONOTONIC, &first);
	clock_gettime(CLOCK_MONOTONIC, &second);

	diff = second.tv_nsec - first.tv_nsec;
	printf("%d\n", diff);
	return 0;
}