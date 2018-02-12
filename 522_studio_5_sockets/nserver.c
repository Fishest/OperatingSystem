#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MY_SOCK_PATH "128.252.167.161"
#define PORT 1025
#define LISTEN_BACKLOG 50
#define READ_BUF_SIZE 80
#define READ_SIZE 10

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	int sfd, cfd;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t peer_addr_size;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		handle_error("socket");
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	if (inet_aton(MY_SOCK_PATH, &(my_addr.sin_addr)) == 0){
		handle_error("inet_aton");
	}
	if (bind(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_in)) == -1) {
		handle_error("bind");
	}
	if (listen(sfd, LISTEN_BACKLOG) == -1)
		handle_error("listen");
	peer_addr_size = sizeof(struct sockaddr_in);
	ssize_t read_size;
	char read_buf[READ_BUF_SIZE];
	while (1) {
		cfd = accept(sfd, (struct sockaddr *) &peer_addr,
          	      &peer_addr_size);
	        if (cfd == -1)
         	       handle_error("accept");
		printf("Reading from a new client\n");
		while (1) {
			read_size = read(cfd, &read_buf, READ_SIZE);
			if (read_size == -1)
				handle_error("read");
			if (read_size == 0)
				break;
			if (strncmp("quit", read_buf, strlen("quit")) == 0){
				printf("Quiting\n");
				unlink(MY_SOCK_PATH);
				return 0;
			}
			printf("Message: %s\n", &read_buf);
		}
	}
	unlink(MY_SOCK_PATH);
	return 0;
}
