#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdatomic.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

enum Status{NOT_WAITING, WAITING_SIG1, WAITING_SIG2};

#define PIPE_BUFF 100

#define FIFO_BUFF 100

#define handle_error(msg) \
	do {printf(msg); exit(EXIT_FAILURE);} while (0)

unsigned int pcount, ccount;

atomic_int p_flag;

void phandler1(int signum) {
	p_flag = WAITING_SIG2;
}

void phandler2(int signum) {
	p_flag = NOT_WAITING;
}

void chandler(int signum) {
	ccount ++;	
}

int main(int argc, char* argv[]) {
	unsigned int comm;
	char* method;
	comm = atoi(argv[1]);
	method = argv[2];
	
	//pipe
	int pipefd[2];
	if (strcmp(method, "pipe") == 0) {
		if (pipe(pipefd) == -1) {
			handle_error("Pipe failed");
		}
	}
	char buf[PIPE_BUFF];





	struct sigaction psigact1;
	psigact1.sa_handler = phandler1;
	psigact1.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR1, &psigact1, NULL) == -1) {
		handle_error("Parent's first sigaction");
	}

	struct sigaction psigact2;
	psigact2.sa_handler = phandler2;
	psigact2.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR2, &psigact2, NULL) == -1) {
		handle_error("Parent's second sigaction");
	}
	
	if (strcmp(method, "FIFO") == 0) {
		if (mkfifo("tmp.file", 0700) == -1) {
			handle_error("FIFO creation");
		}
	}

	p_flag = WAITING_SIG1;
	int cpid;
	cpid = fork();
	if (cpid == -1) {
		handle_error("fork");
	} else if (cpid == 0) {
		printf("Child, with %d times, and %s.\n", comm, method);
		fflush(stdout);
		if (strcmp(method, "signals") == 0) {
			struct sigaction csigact;
			csigact.sa_handler = chandler;
			csigact.sa_flags = SA_RESTART;
			if (sigaction(SIGUSR2, &csigact, NULL) == -1) {
				handle_error("Child in creating sigaction in signals");
			}
		}
		kill(getppid(), SIGUSR1);
		if (strcmp(method, "signals") == 0) {
			while (ccount < comm) {
			}
		}
		if (strcmp(method, "pipe") == 0) {
			close(pipefd[1]);
			int n;
			while (ccount < comm) {
				n = read(pipefd[0], &buf, PIPE_BUFF + 1);
				if (n < 0) {
					handle_error("READ in child process in pipe");
				}
				ccount ++;
			}
		}
		if (strcmp(method, "FIFO") == 0) {
			FILE * rfile = fopen("tmp.file", "r");
			char buf[FIFO_BUFF];
			if (rfile == NULL) {
				handle_error("Child fifo");
			}
			while (ccount < comm) {
				if (fscanf(rfile, "%s", buf) == 0) {
					handle_error("not expected empty child fifo");
				}
				ccount ++;
			}
			fclose(rfile);
		
		}
		
		kill(getppid(), SIGUSR2);

	} else {
		printf("Parent, with %d times, and %s.\n", comm, method);
		fflush(stdout);
		
		while (p_flag == WAITING_SIG1) {
		}
		
		struct timespec st, et;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &st) == -1) {
			handle_error("Get time failed when receiving signal 1");
		}

		if (strcmp(method, "signals") == 0) {
			while (p_flag == WAITING_SIG2) {
				kill(cpid, SIGUSR2);
				pcount++;
			}		
		}
		if (strcmp(method, "pipe") == 0) {
			close(pipefd[0]);
			int i;
			for (i=0; i < PIPE_BUFF; i++){
				buf[i] = (char) 0;
			} 
			while (p_flag == WAITING_SIG2) {
				write(pipefd[1], &buf, PIPE_BUFF + 1);
				pcount++;
			}
			close(pipefd[0]);
		}
			
		if (strcmp(method, "FIFO") == 0) {
			FILE * rfile;
			rfile = fopen("tmp.file", "w");	
			if (rfile == NULL) {
				handle_error("FIFO file Parent");
			}
			char buf[FIFO_BUFF];
			strncpy(buf, "This is for FIFO", sizeof(buf) - 1);
			while (p_flag == WAITING_SIG2) {
				fprintf(rfile, buf);
				pcount++;
			}
			fprintf(stdout, "%d", pcount);
			handle_error("random");
			fclose(rfile);
		}


		if (clock_gettime(CLOCK_MONOTONIC_RAW, &et) == -1) {
			handle_error("Get time failed when receiving signal 2");
		}
		
		fprintf(stdout, "Parent sent %u signals, and it took %ld seconds and %ld nanoseconds", pcount, et.tv_sec - st.tv_sec, et.tv_nsec - st.tv_nsec);
	}	
		

}
