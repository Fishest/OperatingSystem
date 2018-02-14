#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	cpu_set_t *cpuset;
	int cpu;
	if (argc < 2) {
		printf("small");
		return 1;
	}
	printf("here");
	fflush(0);
	cpu = atoi(argv[1]);
	
	printf("here");
	fflush(0);
	cpuset = CPU_ALLOC(4);
	CPU_ZERO(cpuset);

	CPU_SET(cpu, cpuset);
	pid_t pid;
	fflush(0);
	pid = getpid();

	int ret;
	ret = sched_setaffinity(pid, 1, cpuset);
	while (1) {
	}
	return 0;

}