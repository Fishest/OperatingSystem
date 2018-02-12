#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
	if (mkfifo("tmp.file", 0700) == -1) {
		fprintf(stderr, "mkfifo function failed");
	}
	FILE * rFile;
	rFile = fopen("tmp.file", "r");
	if (rFile == NULL) {
		fprintf(stderr, "file not opened");
	}
	//char mystring [100];
	int input;
	/* //This part is for problem 3
	while (fgets(mystring, 100, rFile) != NULL) {
		puts(mystring);
		printf("%s\n", mystring);
	}
	*/

	/* // This part is for problem 4
	fscanf(rFile, "%d", &input);
	printf("%d\n", 2 * input);
	*/

	// This part is for problem 5
	while (fscanf(rFile, "%d", &input) != 0) {
		printf("%d\n", 2 * input);
	}
	fclose(rFile);
	
	return 0;

}