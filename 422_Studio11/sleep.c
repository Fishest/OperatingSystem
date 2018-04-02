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
#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <limits.h>
// 500 million iterations should take several seconds to run
#define ITERS 50000000
#define UNLOCKED 1
#define LOCKED 0


volatile int lock_macro = 1;

void lock(volatile int * input) {
	int ret;
	while (1) {
		ret = __atomic_sub_fetch(input, 1, __ATOMIC_ACQ_REL);
		if (ret < 0) {
			__atomic_store_n(input, -1, __ATOMIC_RELEASE);
			syscall(SYS_futex, input, FUTEX_WAIT, -1, NULL);
		} 
		else {
			break;
		}
	}
}

void unlock(volatile int* input) {
	int ret;
		ret = __atomic_add_fetch(input, 1, __ATOMIC_ACQ_REL);
		if (ret != 1) {
			__atomic_store_n(input, 1, __ATOMIC_RELEASE);
			syscall(SYS_futex, input, FUTEX_WAKE, INT_MAX);
		} 
		
	
}

void critical_section( void ){
	int index = 0;
	while(index < ITERS){ index++; }
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