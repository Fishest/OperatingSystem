/*    Simulated workload using OpenMP
 *
 * This program will create some number of seconds of work on each processor
 * on the system.
 *
 * This program requires the use of the OpenMP compiler flag, and that 
 * optimizations are turned off, to build correctly. E.g.: 
 * gcc -fopenmp workload.c -o workload
 */

#define _GNU_SOURCE
#include <stdio.h> // for printf()
#include <sched.h> // for sched_getcpu()
#include <stdlib.h>
#include <stdbool.h>

// 500 million iterations should take several seconds to run
#define ITERS 1500000000
#define UNLOCKED 1
#define LOCKED 0


volatile int lock_macro = 1;

void lock(volatile int * input) {
	int ret;
	int expected;
	int desired;
	while (1) {
		expected = UNLOCKED;
		desired = LOCKED;
		ret = __atomic_compare_exchange(input, &expected, &desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
		if (ret) {
			break;
		}
		else {
			//printf("Contending\n");
		}
	}
}

void unlock(volatile int* input) {
	int expected = LOCKED;
	int desired = UNLOCKED;
	int ret = __atomic_compare_exchange(input, &expected, &desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	if (!ret) {
		printf("This should never happen\n");
	}
}

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
	printf("FInished.\n");
}

int main (int argc, char* argv[]){

	// Create a team of threads on each processor
	#pragma omp parallel
	{
		// Each thread executes this code block independently
		lock(&lock_macro);
		critical_section();
		unlock(&lock_macro);
		printf("CPU %d finished!\n", sched_getcpu());
	}

	return 0;
}