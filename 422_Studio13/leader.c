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
  int shared_fd = shm_open(SHARED_NAME, O_RDWR | O_CREAT, S_IRWXU);
  if (shared_fd == -1) {
     printf("SHM_OPEN failed\n");
  }
  printf("%u\n", sizeof(struct SharedData));
  if (ftruncate(shared_fd, sizeof(struct SharedData)) != 0) {
    printf("Ftruncate failed\n");
    return -1;
  }
  void * addr = mmap(NULL, sizeof(struct SharedData), PROT_READ|PROT_WRITE, MAP_SHARED, shared_fd, 0);
  if (*(int*)addr == -1) {
    printf("MMap failed\n");
    return -1;
  }
  struct SharedData * sd = (struct SharedData *) addr;
  sd->read_guard = 0;
  sd->write_guard = 0;
  sd->delete_guard = 0;
  volatile int *temp = malloc(sizeof(struct SharedData));
  if (temp == NULL) {
	printf("Out of memory\n");
 	return -1;
  }
  int i;
  for (i = 0; i < SHARED_SIZE; i++) {
    temp[i] = rand();
  }
  printf("Waiting for follower\n");
  while (sd->write_guard == 0) {}
  
  memcpy((void *)(sd->data), (void *)temp, sizeof(int) * SHARED_SIZE);
  
  sd->read_guard = 1;
  printf("waiting for follower to finish reading\n");
  while (sd->delete_guard == 0) {}
  
  if (shm_unlink(SHARED_NAME) == -1) {
	printf("Failed unlinking\n");
  }
  return 0;
}
