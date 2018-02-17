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

#define MY_SOCKET_PATH "127.0.0.1"
#define PORT 1025
#define LISTEN_BACKLOG 50
#define WRITE_BUF_SIZE 80
#define WRITE_SIZE 10

#define TIMEOUT 10
#define BUF_LEN 1

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main (void) {
	
	struct timeval tv;
	fd_set readfds;
	int ret;
	
	int sfd, cfd;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t peer_addr_size;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		handle_error("socket");
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	if (inet_aton(MY_SOCKET_PATH, &(my_addr.sin_addr)) == 0){
		handle_error("inet_aton");
	}
	if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in)) == -1) {
		handle_error("bind");
	}
	if (listen(sfd, LISTEN_BACKLOG) == -1)
		handle_error("listen");
	peer_addr_size = sizeof(struct sockaddr_in);

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	printf("STDIN_FILENO: %d", STDIN_FILENO);
	printf("SOCKET: %d", sfd);
	FD_SET(sfd, &readfds);

	while (1) {
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
		ret = select(sfd + 1, &readfds, NULL, NULL, &tv);

		if (ret == -1) {
			perror("select");
			return 1;
		} else if (!ret) {
			printf("%d seconds elapsed. \n", TIMEOUT);
		}

		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			while (1) {
				char buf[BUF_LEN + 1];
				int len;
				len = read(STDIN_FILENO, buf, BUF_LEN);
				if (len == -1) {
					perror("read");
					return 1;
				}
				if (len) {
					buf[len] = '\0';
					printf("read:%s\n", buf);
				}
				else {
					printf("EOF reached");
		
					return 0;
				}
			}
		} else if (FD_ISSET(sfd, &readfds)) {
			cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
	        	if (cfd == -1)
         	       		handle_error("accept");
			printf("Writing to a new client\n");
			ssize_t write_size;
			char write_buf[WRITE_BUF_SIZE] = "tencharac";
			write_size = write(cfd, &write_buf, WRITE_SIZE);
			if (write_size == -1)
				printf("First write error, reason: %s\n", strerror(errno));
		}
	}
	return 1;
}
