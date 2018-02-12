#include <stdio.h>
#include <unistd.h>

void genChildren(int generationLevel);

int generations;

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Too few arguments provided. %s <generation>", argv[0]);
		return 1;
	}
	generations = atoi(argv[1]);
	genChildren(1);
	return 0;
}

void genChildren(int generation) {
	printf("pid: %d, generation: %d\n", getpid(), generation);
	if (generation == generations) {
		return;
	}
	pid_t id;
	id = fork();
	if (id == -1) {
		printf("fork failed");
		
	} else if (id == 0) {
		genChildren(generation + 1);
	} else{
	id = fork();
	if (id == -1) {
		printf("fork failed");
		
	} else if (id == 0) {
		genChildren(generation + 1);
	}};
	wait(0);
}