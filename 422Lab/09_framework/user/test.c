#include <sched_monitor.h>
#include <stdio.h>

int main() {
   int size;
   size = sizeof(struct preemption_info);
   printf("size: %d\n", size);
   return 0;
}


