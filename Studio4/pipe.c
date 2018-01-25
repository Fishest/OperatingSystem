#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

int main()
{
	int fd[2];
	char fixed_str[] = "studio4 for cse522";
	char input_str[100];
	pid_t p;
	if (pipe(fd) == -1) {
		fprintf(stderr, "Pipe Failed");
		return 1;
	}
	p = fork();
	if (p < 0) {
		fprintf(stderr, "Fork Failed");
		return 1;
	} else if (p == 0) {
		close(fd[0]);
		//printf("closed");
		write(fd[1], fixed_str, strlen(fixed_str)+1);
		//printf("write");
	} else {
		close(fd[1]);
		//printf("closed read");
		char rbuf[100];
		read(fd[0], rbuf, 100);
		//printf("read");
		printf("%s\n",rbuf);
	}
	return 0;

}
