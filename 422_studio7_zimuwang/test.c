#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pid.h>

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Too few arguments provided. %s <pid>", argv[0]);
		return 1;
	}
	int pid;
	pid = atoi(argv[1]);
	struct pid* vpid;
	vpid = find_vpid(pid);
	return 0;
}

