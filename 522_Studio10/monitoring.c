#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>


#define TIMEOUT 10000

int main(int argc, char** argv) {
	char* cgroup_file1 = argv[1];
	char* memory_file1 = argv[2];
	char* cgroup_file2 = argv[3];
	char* memory_file2 = argv[4];
	int fd1 = eventfd(0, 0);
	int fd2 = eventfd(0, 0);
	struct pollfd fds[3];
	fds[0].fd = fd1;
	fds[0].events = POLLIN;
	fds[1].fd = fd2;
	fds[1].events = POLLIN;
	printf("Opening %s\n", cgroup_file1);
	int cgroup_fd1 = open(cgroup_file1, O_WRONLY);
	if (cgroup_fd1 < 0) {
		printf("Cgroup file can not be opened\n");
	}
	int memory_fd1 = open(memory_file1, O_RDONLY); 
	if (memory_fd1 < 0) {
		printf("memory file can not be opened\n");
	}
	printf("Opening %s\n", cgroup_file2);
	int cgroup_fd2 = open(cgroup_file2, O_WRONLY);
	if (cgroup_fd2 < 0) {
		printf("Cgroup file can not be opened\n");
	}
	int memory_fd2 = open(memory_file2, O_RDONLY); 
	if (memory_fd2 < 0) {
		printf("memory file can not be opened\n");
	}
	char s[11]; 

	sprintf(s,"%d %d", fd1, memory_fd1);
	printf("first string %s\n", s);
	int ret = write(cgroup_fd1, s, strlen(s));
	if (ret == -1) {
		printf("write failed, %s\n", strerror(errno));
		printf("cgroup fd: %d, %s, %d\n", cgroup_fd1, s, strlen(s));
	}
	sprintf(s,"%d %d", fd2, memory_fd2);
	printf("first string %s\n", s);
	ret = write(cgroup_fd2, s, strlen(s));
	if (ret == -1) {
		printf("write failed, %s\n", strerror(errno));
		printf("cgroup fd: %d, %s, %d\n", cgroup_fd2, s, strlen(s));
	}
	char buff[100];
	while (1) {
		ret = poll(fds, 2, TIMEOUT);

		if (ret == -1) {
			perror("pool");
			return 1;
		} else if (ret == 0) {
			printf("%d seconds elapsed. \n", TIMEOUT);
		}
		if (ret > 0) {	
			if (fds[0].revents != 0) {
				int read_ret = read(fd1, buff, 10);
				if (read_ret != 0) {
					unsigned int result;
					sscanf(buff, "%u", result);
					printf("Message received %u\n", result);
					break;
				}
			}
			else if (fds[1].revents != 0) {
				int read_ret = read(fd2, buff, 10);
				if (read_ret != 0) {
					unsigned int result;
					sscanf(buff, "%u", result);
					printf("Message received %u\n", result);
					break;
				}
			}
		}
	}
		
}
