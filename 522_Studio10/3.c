#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	void* data = malloc(10);
	
	strcpy(data, "aaaaaaaaaaaaaaaaaa");
	
}
