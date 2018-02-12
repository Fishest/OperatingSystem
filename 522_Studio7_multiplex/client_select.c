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
#define READ_BUF_SIZE 80
#define READ_SIZE 10

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
	ssize_t read_size;
	char read_buf[READ_BUF_SIZE];
	while (1) {
		read_size = read(sfd, &read_buf, READ_SIZE);
		if (read_size == -1) {
			handle_error("read failed");
		} else if (read_size == 0) {
			break;
		}
		printf("Message: %s\n", &read_buf);
	}
	unlink(MY_SOCKET_PATH);
	return 0;
}
