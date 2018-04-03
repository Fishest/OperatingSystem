#include <stdlib.h>

const int SHARED_SIZE = 10
const char* SHARED_NAME = "shared_memory"

struct SharedData {
  volatile int write_guard;
  volatile int read_guard;
  volatile int delete_guard;
  volatile int data[SHARED_SIZE];
};
