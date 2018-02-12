#include <stdio.h>
#include <time.h>

int main()
{
	struct timespec time;
	
	// Using the first four types of clocks on the man page

	if (clock_getres( CLOCK_REALTIME, &time))
	{
		printf("Error: Realtime failed\n");
	}
	printf( "CLOCK_REALTIME: seconds=%d, nano_sec=%d\n", time.tv_sec, time.tv_nsec);

	if (clock_getres( CLOCK_REALTIME_COARSE, &time))
	{
		printf("Error: Realtime Coarse failed\n");
	}
	printf( "CLOCK_REALTIME_COARSE: seconds=%d, nano_sec=%d\n", time.tv_sec, time.tv_nsec);

	if (clock_getres( CLOCK_MONOTONIC, &time))
	{
		printf("Error: Monotonic failed\n");
	}
	printf( "CLOCK_MONOTONIC: seconds=%d, nano_sec=%d\n", time.tv_sec, time.tv_nsec);

	if (clock_getres( CLOCK_MONOTONIC_COARSE, &time))
	{
		printf("Error: Monotonic failed\n");
	}
	printf("CLOCK_MONOTONIC_COARSE: seconds=%d, nano_sec=%d\n", time.tv_sec, time.tv_nsec);

	return 0;
}