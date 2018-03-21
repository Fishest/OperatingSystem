#include <stdio.h>

typedef unsigned long long U64;

struct preemption_info{
  U64 start;
  U64 end;
  U64 duration;
  int core;
  char* name;
}__attribute__((packed));

int main() {
  printf("size: %u", sizeof(struct preemption_info));
}
