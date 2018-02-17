#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char* argv[]) {
	cpu_set_t *cpuset;
	int cpu;
	if (argc < 3) {
		printf("small");
		return 1;
	}
	
	cpu = atoi(argv[1]);
	int priority = atoi(argv[2]);
	if (priority < sched_get_priority_min(SCHED_RR) || priority > sched_get_priority_max(SCHED_RR)) {
		printf("out of range");
		return -1;
	}
	cpuset = CPU_ALLOC(4);
	CPU_ZERO(cpuset);

	CPU_SET(cpu, cpuset);
	pid_t pid;
	
	pid = getpid();

	int ret;
	ret = sched_setaffinity(pid, 1, cpuset);
	
	struct sched_param param;
	param.sched_priority = priority;
	if (sched_setscheduler(0, SCHED_RR, &param) != 0) {
		printf("failed to set");
		return -1;
	}
	long long i;
	
	while (i < 500000000) {
		i++;
	}
	return 0;

}