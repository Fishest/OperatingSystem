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
  void * addr = mmap(NULL, sizeof(SharedData), PROT_READ|PROT_WRITE, MAP_SHARED, shared_fd, 0);
  if (*(int*)addr == -1) {
    printf("MMap failed\n");
    return -1;
  }
  struct SharedData * sd = (struct SharedData *) addr;

  for (i = 0; i < SHARED_SIZE; i++) {
    printf("%d ", sd->data[i]);
  }
}
