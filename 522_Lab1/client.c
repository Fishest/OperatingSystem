#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#define LINE_BUF 1024
#define MY_SOCK_PATH "127.0.0.1"
#define LISTEN_BACKLOG 50
#define BUF_LEN 1024
#define TIMEOUT 5
#define READ_SIZE 5
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
};

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
	struct text *text_root;
	struct line *line_root;
	text_root = (struct text *)malloc(sizeof(struct text));
	line_root = (struct line *)malloc(sizeof(struct line));
	struct text *current_node=text_root;
	struct line *current_line = line_root;

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
	ssize_t read_s;

  char readin_buff[BUF_LEN];
  
  while (1) {
    read_s = read(sfd, &readin_buff, READ_SIZE);
    readin_buff[read_s] = '\0';
    printf("message: %s, read_size: %d\n", readin_buff, read_s);
    if (read_s == -1)
      handle_error("read");
    char* cur_fragment = (char*)malloc(read_s + 1);
	
    	strcpy(cur_fragment, readin_buff);
	
	struct text * new_node = (struct text *)malloc(sizeof(struct text));
	new_node->fragment = cur_fragment;
	
	new_node->length = read_s;
	current_node->next = new_node;
	new_node->next = NULL;
	current_node = new_node;
	printf("Current fragment: %s, current_length: %d\n", new_node->fragment, new_node->length);
    int i;
    int is_EOL;
    is_EOL = 0;
    for (i = 0; i<read_s; i++) {
	printf("%d-th: %c\n", i, readin_buff[i]);
        if (readin_buff[i] == '\n') {
		is_EOL = 1;
		break;
	}
    }
    if (is_EOL) {
	    // Signals end of line
	    // Create new line
    	int count;
	count = 0;
	struct text* head = text_root;
	struct text* next;
	while (head->next != NULL) {
		head = head->next;
		printf("Fragment: %s, count: %d, total: %d\n", head->fragment, head->length, count); 
		count += head->length;
	}
	printf("Count: %d\n", count);
	char* cur_line = (char*)malloc(count + 1);
	head = text_root;
	printf("Reached here");
	fflush(stdout);
	while (head->next != NULL) {
		printf("Message: %s\n", head->fragment);
		fflush(stdout);
		head = head->next;
		//free(head);
		//head = next;
		strcat(cur_line, head->fragment);
		
	}
	printf("Reached concat\n");
	fflush(stdout);
	struct line* new_line = (struct line *)malloc(sizeof(struct line));
	new_line->line = cur_line;
	printf("Entire line read: %s\n", cur_line);
	new_line->next = NULL;
	current_line->next = new_line;
	current_node = text_root;
	current_node->next = NULL;
	
    } 
  }

	return 0;
}
