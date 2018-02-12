#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

#define TIMEOUT 10000
#define BUF_LEN 1

int main (void) {
	
	int epoll;
	int ret;
	epoll = epoll_create1(0);
	if (epoll == -1) {
		printf("epoll failed");
	}
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET;

	ret = epoll_ctl(epoll, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	
	if (ret == -1) {
		perror("epoll_ctl");
		return 1;
	} 
	printf("Something returned");
	while (1) {
		ret = epoll_wait(epoll, &event, 5, TIMEOUT); 
		
		if (ret == -1) {
			printf("epoll_wait failed");
		} else if (ret > 0) {
			printf("data available");
		} else {
			perror("empty readin");
			return 1;
		}
	}
	return 1;
}