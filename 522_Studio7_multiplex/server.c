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

#define MY_SOCKET_PATH "127.0.0.1"
#define PORT 1025
#define LISTEN_BACKLOG 50
#define READ_BUF_SIZE 80
#define READ_SIZE 10

#define TIMEOUT 10000
#define BUF_LEN 1
#define FD_SIZE 3

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

	struct pollfd fds[FD_SIZE];
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[1].fd = sfd;
	fds[1].events = POLLIN;
	while (1) {
		ret = poll(fds, 2, TIMEOUT);

		if (ret == -1) {
			perror("pool");
			return 1;
		} else if (ret == 0) {
			printf("%d seconds elapsed. \n", TIMEOUT);
		}

		if (ret > 0) {	
			printf("ret: %d", ret);
			if (fds[0].revents != 0) {
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
					printf("EOF reached\n");
					fds[0].fd = -1;
				}
				break;
			}
			} else if (fds[1].revents != 0) {
			printf("Client request received\n");
			while (1) {
			cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
	        	if (cfd == -1)
         	       		handle_error("accept");

			fds[2].fd = cfd;
			fds[2].events = POLLIN;
			while (1) {
				ret = poll(fds, 3, TIMEOUT);

				if (ret == -1) {
					perror("pool");
					return 1;
				}
				if (ret > 0 && fds[2].revents != 0) {
					int read_size;
					char read_buf[READ_BUF_SIZE];
					read_size = read(cfd, &read_buf, READ_SIZE);
					if (read_size == -1) {
						handle_error("read failed");
					} else if (read_size == 0) {
						printf("EOF reached\n");
						fds[2].fd = -1;
						break;
					}
					if (strncmp("quit", read_buf, strlen("quit")) == 0){
						printf("Quiting\n");
						unlink(MY_SOCKET_PATH);
						return 0;
					}
					printf("Message: %s\n", &read_buf);

				}
			}
		}

		}

			
	}
	}
	unlink(MY_SOCKET_PATH);
	return 1;
}
