#include <stdlib.h>

#define SHARED_SIZE 100000000
const char* SHARED_NAME = "shared_memory";

struct SharedData {
  volatile int write_guard;
  volatile int read_guard;
  volatile int delete_guard;
  volatile int data[SHARED_SIZE];
};
