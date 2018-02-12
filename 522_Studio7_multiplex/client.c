#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MY_SOCKET_PATH "127.0.0.1"
#define PORT 1025
#define WRITE_BUF_SIZE 80
#define WRITE_SIZE 10

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	int sfd;
	struct sockaddr_in my_addr;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		printf("Socket error, reason: %s\n",
			strerror(errno));
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);

	my_addr.sin_addr.s_addr = inet_addr(MY_SOCKET_PATH);
	fprintf(stderr, "Before connect\n");
	if (connect(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_in)) == -1) {
		printf("Connect error, reason: %s\n", 
                        strerror(errno));
	}
	fprintf(stderr, "After connect\n");
	printf("Writing to a server\n");
	ssize_t write_size;
	char write_buf[WRITE_BUF_SIZE] = "tencharac";
	char quit_buf[WRITE_BUF_SIZE] = "quit";
	write_size = write(sfd, &write_buf, WRITE_SIZE);
	if (write_size == -1)
		printf("First write error, reason: %s\n", strerror(errno));
	write_size = write(sfd, &write_buf, WRITE_SIZE);
	if (write_size == -1)
		printf("Second write error, reason: %s\n", strerror(errno));
	write_size = write(sfd, &quit_buf, WRITE_SIZE);
	if (write_size == -1)
		printf("Third write error, reason: %s\n", strerror(errno));
	//unlink(MY_SOCKET_PATH);
	return 0;
}
