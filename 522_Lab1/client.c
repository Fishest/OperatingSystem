#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/types.h>

#define LINE_BUF 1024
#define MY_SOCK_PATH "127.0.0.1"
#define LISTEN_BACKLOG 50
#define BUF_LEN 1024
#define TIMEOUT 5

#define handle_error(msg) \
	do{perror(msg);exit(EXIT_FAILURE);}while(0)

struct text{
	char* fragment;
  int length;
	struct text *next;
};

struct line{
  int number;
  char* line;
  struct line *next;
}

int main(int argc, char*argv[]){
	char* address;
	int port;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s internet_address port\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[2]);
	address = argv[1];


	int count;
	struct text *root;
	root = (struct text *)malloc(sizeof(struct text));
	struct text *current_node=root;

  int sfd;
	struct sockaddr_in my_addr;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		printf("Socket error, reason: %s\n",
			strerror(errno));

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);


	my_addr.sin_addr.s_addr = inet_addr(address);
	fprintf(stderr, "Before connect\n");
	if (connect(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_in)) == -1) {
		printf("Connect error, reason: %s\n",
                        strerror(errno));
	}
	fprintf(stderr, "After connect\n");
	ssize_t read_size;

  char* readin_buff[BUF_LEN];

  while (1) {
    read_size = read(cfd, &read_buf, READ_SIZE);
    if (read_size == -1)
      handle_error("read");
    if (read_size == 0)
      break;
    printf("Message: %s\n", &read_buf);
  }
	return 0;
}
