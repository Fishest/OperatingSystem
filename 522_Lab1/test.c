#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/types.h>


int main() {
  char readin[100];
  int n;
  n = read(0, &readin, 5);
  int i;
  for (i = 0; i < n; i++) {
    if (readin[i] != '\n')
      printf("%c: %d\n", readin[i], i);
  }
  printf("%s: %d\n", readin, n);
  return 0;
}
