#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MY_SOCKET_PATH "home/pi/Desktop/socket"
#define LISTEN_BACKLOG 50
#define READ_BUF_SIZE 80
#define READ_SIZE 10

#define handle_error(msg) \
	do {perror(msg); exit(EXIT_FAILURE);} while (0)

int main(int argc, char* argv[]) {
	int sfd, cfd;
	struct sockaddr_un my_addr, peer_addr;
	socklen_t peer_addr_size;
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1) {
		handle_error("socket");
	}
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path, MY_SOCKET_PATH, sizeof(my_addr.sun_path) - 1);
	if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1) {
		handle_error("bind");
	}
	if (listen(sfd, LISTEN_BACKLOG) == -1) {
		handle_error("listen");
	}
		

}
