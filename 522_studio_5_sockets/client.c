#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MY_SOCK_PATH "/home/pi/studio5/socket"
#define WRITE_BUF_SIZE 80
#define WRITE_SIZE 10

int main(int argc, char *argv[]) {
	if (argc > 2) {
		printf("Usage: %s [quit]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int sfd, cfd;
	struct sockaddr_un my_addr, peer_addr;
	socklen_t peer_addr_size;
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd == -1)
		printf("Socket error, reason: %s\n",
			strerror(errno));
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	strncpy(my_addr.sun_path, MY_SOCK_PATH,
		sizeof(my_addr.sun_path)-1);
	if (connect(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_un)) == -1) {
		printf("Connect error, reason: %s\n", 
                        strerror(errno));
	}
	ssize_t write_size;
	if (argc == 2 && strncmp("quit",argv[1],strlen("quit")) == 0) {
		char write_buf[WRITE_BUF_SIZE] = "quit";
        	write_size = write(sfd, &write_buf, WRITE_SIZE);
        	if (write_size == -1)
                	printf("Quit write error, reason: %s\n", 
                        	strerror(errno));
		return 0;
	}
	char write_buf[WRITE_BUF_SIZE] = "tencharac";
	write_size = write(sfd, &write_buf, WRITE_SIZE);
	if (write_size == -1)
		printf("First write error, reason: %s\n", 
                        strerror(errno));
	char write_buf2[WRITE_BUF_SIZE] = "somemoree";
	write_size = write(sfd, &write_buf2, WRITE_SIZE);
        if (write_size == -1)
                printf("Second write error, reason: %s\n", 
                        strerror(errno));
	//unlink(MY_SOCK_PATH);
	return 0;
}
