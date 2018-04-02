#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

int main(int argc, char** argv) {
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	int i;
	int core;
	for (i = 1; i < argc; i++) {
		core = (int) atoi(argv[i]);
		CPU_SET(core, &cpuset);
	}
	sched_setaffinity(getpid(), argc - 1, &cpuset);
	printf("Process id: %d\n", (int)getpid());
	for (i = 0; i < argc; i++) {
		printf("Argument %d-th: %s\n", i, argv[i]);
	}
	while (1) {
		i *= 2;
	}
}