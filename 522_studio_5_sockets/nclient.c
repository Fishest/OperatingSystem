#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MY_SOCK_PATH "128.252.167.161"
#define PORT 1025
#define WRITE_BUF_SIZE 80
#define WRITE_SIZE 10

int main(int argc, char *argv[]) {
	if (argc > 2) {
		printf("Usage: %s [quit]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int sfd;
	struct sockaddr_in my_addr;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		printf("Socket error, reason: %s\n",
			strerror(errno));
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	//if (inet_aton(MY_SOCK_PATH, &(my_addr.sin_addr.s_addr)) == 0){
        //        printf("inet_aton error");
        //}
	my_addr.sin_addr.s_addr = inet_addr(MY_SOCK_PATH);
	fprintf(stderr, "Before connect\n");
	if (connect(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_in)) == -1) {
		printf("Connect error, reason: %s\n", 
                        strerror(errno));
	}
	fprintf(stderr, "After connect\n");
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
