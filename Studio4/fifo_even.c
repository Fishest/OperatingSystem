#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
	
	FILE * rFile;
	rFile = fopen("tmp.file", "w");
	if (rFile == NULL) {
		fprintf(stderr, "file not opened");
	}
	int count = 0;
	while (count <= 100) {
		fprintf(rFile, "%d\n", 100);
		
	}
	
	fclose(rFile);
	
	return 0;

}