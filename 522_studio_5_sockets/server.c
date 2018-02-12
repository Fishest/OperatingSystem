#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MY_SOCK_PATH "/home/pi/studio5/socket"
#define LISTEN_BACKLOG 50
#define READ_BUF_SIZE 80
#define READ_SIZE 10

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	int sfd, cfd;
	struct sockaddr_un my_addr, peer_addr;
	socklen_t peer_addr_size;
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1)
		handle_error("socket");
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path, MY_SOCK_PATH,
		sizeof(my_addr.sun_path)-1);
	if (bind(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_un)) == -1) {
		handle_error("bind");
	}
	if (listen(sfd, LISTEN_BACKLOG) == -1)
		handle_error("listen");
	peer_addr_size = sizeof(struct sockaddr_un);
	//cfd = accept(sfd, (struct sockaddr *) &peer_addr,
	//	&peer_addr_size);
	//if (cfd == -1)
	//	handle_error("accept");
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
