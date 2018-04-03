#include <stdlib.h>

#define SHARED_SIZE 10
#define SHARED_NAME "shared_memory"

struct SharedData {
  volatile int write_guard;
  volatile int read_guard;
  volatile int delete_guard;
  volatile int data[SHARED_SIZE];
};
