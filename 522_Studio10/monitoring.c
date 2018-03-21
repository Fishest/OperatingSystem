#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** argv) {
	char* cgroup_file = argv[1];
	char* memory_file = argv[2];
	int fd;
	fd = eventfd(0, 0);
	int cgroup_fd = open(cgroup_file, O_RDWR);
	int memory_fd = open(memory_file, O_RDONLY);
	char s[11]; 

	sprintf(s,"%d", fd);
	write(cgroup_fd, s, strlen(s));

	s[0] = ' ';
	s[1] = '\0';
	write(cgroup_fd, s, 1);

	sprintf(s,"%d", memory_fd);
	write(cgroup_fd, s, strlen(s));
	char buff[100];
	while (0) {
		int ret = read(fd, buff, 64);
		if (ret != 0) {
			unsigned int result;
			sscanf(buff, "%u", result);
			printf("Message received %u", result);
		}
	}
		
}
