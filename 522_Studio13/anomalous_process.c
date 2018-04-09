#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>

#define SPIN 1

int keep_forking(char* program_name, int seconds){
	pid_t cpid;
	printf("program name is %s pid is %ld.\n",program_name, getpid());
	sleep(seconds);
	cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (cpid == 0){
		keep_forking(program_name, seconds);
	}else{
		keep_forking(program_name, seconds);
	}
	return 0;

}

int random_fork(char* program_name, int seconds, int forkp, int terminate){
	pid_t cpid;
	int random;
	printf("program name is %s pid is %ld.\n",program_name, getpid());
	sleep(seconds);
	while(SPIN){
		random = rand()%100+1;
		if (random <= forkp){
			cpid = fork();
			if (cpid == -1) {
				perror("fork");
				exit(EXIT_FAILURE);
			}
			if (cpid == 0){
				printf("process %ld forks.\n", getpid());
				random_fork(program_name, seconds, forkp, terminate);
				break;
			}
		}
		random = rand()%100+1;
		if (random <= terminate) {
			printf("process %ld terminates.\n", getpid());
			break;
		}
	}
	return 0;

}

int main(int argc, char* argv[]){
	char* procedure;
	int seconds, fork, terminate;
	pid_t cpid, w;
	if (argc != 5){
		printf("Usage: %s [procedure] [seconds] [fork] [terminate]\n", argv[0]);
		return -1;
	}
	procedure = argv[1];
	seconds = atoi(argv[2]);
	fork = atoi(argv[3]);
	terminate = atoi(argv[4]);
	if (fork <0 || fork > 100) {
		printf("fork probability not within range.\n");
		return -1;
	}
	if (terminate < 0 || terminate > 100){
		printf("terminate probability not within range.\n");
		return -1;
	}
	printf("procedure is %s for %d seconds.\n",
		procedure, seconds);
	
	if (strncmp(procedure, "slowbranch", 10)==0){
		printf("slowbranch.\n");
		keep_forking(argv[0],seconds);
	}else if (strncmp(procedure, "randombranch", 12)==0){
		printf("randombranch.\n");
		srand((unsigned) time(0));
		random_fork(argv[0], seconds, fork, terminate);
	}else{
		printf("procedure not available.\n");
		return -1;
	}
	
	return 0;
}
