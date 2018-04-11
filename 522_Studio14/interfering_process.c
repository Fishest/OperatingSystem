#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

#define PRIORITY 10



void timer_handler(int signum)
{
   (void) signum;       /*  Avoids warning for unused argument  */
   printf("Signal catched\n");
}

void work(int workload) {
  printf("Start working\n");
  int i = 0;
  int temp = 1;
  for (i; i < workload; i++) {
    temp *= 2;
  }
}

int main(int argc, char* argv[]){
  char* scheduler;
	int core, period, iterations;
	if (argc != 5){
		printf("Usage: %s [scheduler] [core] [period] [iterations]\n", argv[0]);
		return -1;
	}

  cpu_set_t cpuset;
	CPU_ZERO(&cpuset);

  core = atoi(argv[2]);
  CPU_SET(core, &cpuset);
  sched_setaffinity(getpid(), 1, &cpuset);

  struct sched_param param;
	param.sched_priority = PRIORITY;
  int policy = 0;
  if (strncmp(argv[1], "RR", 2) == 0) {
    policy = SCHED_RR;
  }
  else if (strncmp(argv[1], "FIFO", 4) == 0) {
    policy = SCHED_FIFO;
  }
  else if (strncmp(argv[1], "OTHER", 5) == 0) {
    policy = SCHED_OTHER;
  }
  else {
    printf("Policy unknown");
    return -1;
  }
  if (sched_setscheduler(getpid(), policy, &param) != 0) {
		printf("failed to set");
		return -1;
	}
  printf("Setting up timer\n");
  struct sigaction sa;
  sa.sa_handler = timer_handler;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGALRM, &sa, NULL);

  struct itimerval timer;

  period = atoi(argv[3]);
  iterations = atoi(argv[4]);
  printf("Period: %d\n", period);
  timer.it_value.tv_sec = period;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
     printf("Setitimer failed\n");
  }
  printf("Start to perform work\n");

  while (1) {
    pause();
    timer.it_value.tv_sec = period;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
    work(iterations);
  }

}
