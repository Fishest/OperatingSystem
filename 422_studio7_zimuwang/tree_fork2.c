#include <stdio.h>
#include <unistd.h>



int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Too few arguments provided. %s <generation>", argv[0]);
		return 1;
	}
	int generations;
	generations = atoi(argv[1]);
	
	int gen;
	gen = 1;
	while (gen <= generations) {
		printf("pid: %d, generation: %d\n", getpid(), gen);
	
		pid_t id;
		id = fork();
		if (id == -1) {
			printf("fork failed");
		
		} else if (id == 0) {
			gen++;
			continue;
		} else{
			id = fork();
			if (id == -1) {
				printf("fork failed");
		
			} else if (id == 0) {
				gen++;
				continue;
			}
		}
		wait(0);
		break;
	}
}