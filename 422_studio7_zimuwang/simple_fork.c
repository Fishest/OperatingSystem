#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
	pid_t ret;
	
	
	printf("this is the parent process\n");
	ret = fork();
	if (ret == -1) {
		printf("fork failed");
	} else if (ret == 0) {
		printf("child with %d id and %d parent id\n", getpid(), getppid());
	} else {
		printf("parent with %d child id\n", ret);
	}
	return 0;
}
