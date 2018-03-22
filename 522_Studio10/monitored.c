#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	
	void* data;
	while (1) {
		data = malloc(1000);
		if (data != NULL) {
			//printf("going");
		} else {
			printf("abort");
		}
	}
	return 0;
}
