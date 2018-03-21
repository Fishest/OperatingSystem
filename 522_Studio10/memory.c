#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	void* data = malloc(200);
	struct mallinfo info;
	info = mallinfo();
	printf("arena: %d, ordblks: %d, smblks: %d, hblks: %d, hblkhd: %d, usmblks: %d, fsmblks: %d, uordblks: %d, fordblks: %d, keepcost: %d \n", info.arena, info.ordblks, info.smblks, info.hblks, info.hblkhd, info.usmblks, info.fsmblks, info.uordblks, info.fordblks, info.keepcost);
	free(data);
	info = mallinfo();
	printf("arena: %d, ordblks: %d, smblks: %d, hblks: %d, hblkhd: %d, usmblks: %d, fsmblks: %d, uordblks: %d, fordblks: %d, keepcost: %d \n", info.arena, info.ordblks, info.smblks, info.hblks, info.hblkhd, info.usmblks, info.fsmblks, info.uordblks, info.fordblks, info.keepcost);
	return 0;
}
