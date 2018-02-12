#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int i = 0;
	for (i = 0; i < 10; ++i) {
		malloc(1000000);
	}
	return 0;
}
