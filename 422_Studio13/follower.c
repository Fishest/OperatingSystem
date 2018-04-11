#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "header.h"

int main() {
  srand(0);
  int shared_fd = shm_open(SHARED_NAME, O_RDWR, S_IRWXU);
  void * addr = mmap(NULL, sizeof(struct SharedData), PROT_READ|PROT_WRITE, MAP_SHARED, shared_fd, 0);
  if (*(int*)addr == -1) {
    printf("MMap failed\n");
    return -1;
  }
  struct SharedData * sd = (struct SharedData *) addr;
  sd->write_guard = 1;
  printf("Waiting for leader\n");
  while (sd->read_guard ==0) {}
  
  volatile int *temp = malloc(sizeof(struct SharedData));
  if (temp == NULL) {
	printf("Out of memory\n");
 	return -1;
  }
  memcpy((void *) temp, (void *)(sd->data), sizeof(int) * SHARED_SIZE);
  sd->delete_guard = 1;
  return 0;
}
