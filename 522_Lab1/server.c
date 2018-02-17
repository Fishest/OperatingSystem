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
#define READ_SIZE 100
#define FILE_READ_SIZE 5

#define handle_error(msg) \
	do{perror(msg);exit(EXIT_FAILURE);}while(0)

struct node{
	FILE* file;
	struct node *next;
	int cfd;
	int allsent;
};

int main(int argc, char*argv[]){
	char* input_filename, output_filename;
	int port;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s filename port\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	port = atoi(argv[2]);
	input_filename = argv[1];

	FILE* f_in;
	FILE* f_out;
  // Input file
  f_in = fopen(input_filename, "r");
	if (f_in == NULL) {
		fprintf(stderr, "Cannot open file %s\n", input_filename);
	}

	int count;
	struct node *root;
	root = (struct node *)malloc(sizeof(struct node));
	struct node *current_node=root;
	char line[LINE_BUF];
	int n;

  // Output file
  n = fscanf(f_in, "%[^\n]\n", &line);
  f_out = fopen(line, "w");
  if (f_out == NULL) {
    fprintf(stderr, "Cannot open file for output %s\n", output_filename);
  }
  // Set the value of root
  root->file = f_out;
  root->cfd = -1;
  // Read in all the file names
	while(1){
		n = fscanf(f_in,"%[^\n]\n",&line);
		if (n==EOF) break;
		if (n==-1) {
			fprintf(stderr,"Cannot read line %d\n", count);
			handle_error("read");
		}
		printf("line %d: %s\n",count,&line);
		FILE* file=fopen(line,"r");
		if(file==NULL){
			handle_error("fopen");
		}
		struct node *new_node=(struct node *)malloc(sizeof(struct node));
		new_node->next = NULL;
		new_node->file = file;
		new_node->allsent = 0; //false
		current_node->next=new_node;
		current_node=current_node->next;
		count++;
	}
	//tie into a circle
	current_node->next = root;
	current_node=root->next;

	/*create and bind socket*/
	int sfd, cfd;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t peer_addr_size;

  	struct timeval tv;
	fd_set readfds;
	int select_ret;
	int biggest_fd;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		handle_error("socket");
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	if (inet_aton(MY_SOCK_PATH, &(my_addr.sin_addr)) == 0){
		handle_error("inet_aton");
	}
	if (bind(sfd, (struct sockaddr *) &my_addr,
		sizeof(struct sockaddr_in)) == -1) {
		handle_error("bind");
	}
	if (listen(sfd, LISTEN_BACKLOG) == -1)
		handle_error("listen");
	printf("Address: %s\n",MY_SOCK_PATH);
	printf("Port: %d\n",port);
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	FD_SET(sfd, &readfds);

	if(sfd>STDIN_FILENO){
		biggest_fd=sfd;
	}else{
		biggest_fd=STDIN_FILENO;
	}
	int cur_num_connections;
	cur_num_connections = 0;
	struct node* cur_connection = root->next;
	printf("biggest_fd: %d", biggest_fd);
	struct node * cur_sending = root->next;
	while (1) {
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(sfd, &readfds);
		// set all the fds of file handler
		struct node* first = root->next;
		while (first->next != root) {
			first = first->next;
			FD_SET(first->cfd, &readfds);
			if (first->cfd > biggest_fd) {
				biggest_fd = first->cfd;
			};
		}

		tv.tv_sec = 0;
		tv.tv_usec = TIMEOUT;
		select_ret=select(biggest_fd+1,&readfds,NULL,NULL,&tv);
		if(select_ret==-1){
			handle_error("select");
		}
		if(FD_ISSET(STDIN_FILENO, &readfds)){
			char buf[BUF_LEN+1];
			int len;
			len=read(STDIN_FILENO, buf, BUF_LEN);
			if(len==-1){
				handle_error("read");
			}
			if(len){
				buf[len]='\0';
				printf("read: %s",buf);
			}
		}
    		// New connections
		if(FD_ISSET(sfd, &readfds) && cur_num_connections < count){
			ssize_t write_size;
			peer_addr_size = sizeof(struct sockaddr_in);
			char write_buf[BUF_LEN];
			char message[] = "hostname: mhost";
			strncpy(write_buf,message,sizeof(message));
			fprintf(stderr, "Before accept\n");
			cfd = accept(sfd, (struct sockaddr *) &peer_addr,
        			&peer_addr_size);
			cur_connection->cfd = cfd;
			fprintf(stderr, "After accept\n");
			if (cfd == -1)
				handle_error("accept");
			fprintf(stdout, "things to write %s\n",message);
			printf("message size: %d\n",strlen(message));
			//write_size = write(cfd, &message, strlen(message));
			fprintf(stdout, "write size: %d\n",write_size);
			if (write_size == -1)
				handle_error("write");

			fprintf(stdout, "Opened connection # %d", cur_num_connections);
			cur_num_connections++;
			cur_connection = cur_connection->next;
		}
		//write to clients
		if (cur_sending->cfd > 0 && !cur_sending->allsent) {
			char write_buf[BUF_LEN];
			char* ret;
			int size;
			int num_char_write;
			ret = fgets(write_buf, FILE_READ_SIZE, cur_sending->file);
			if (ret == NULL) {
				fprintf(stderr, "reading failed or EOF reached");
				cur_sending->allsent = 1;
				//send signal to process
			} else {
				size = strlen(write_buf);
				num_char_write = write(cur_sending->cfd, &write_buf, size);
				if (num_char_write == -1) {
					fprintf(stderr, "writing failed");
				} 
			}
			
		}
		cur_sending = cur_sending->next;
		struct node* cur_checking = root;
		while (cur_checking->next != root) {
			cur_checking = cur_checking->next;
			// If there is anything to read
			if (FD_ISSET(cur_checking->cfd, &readfds)) {
				char read_buf[BUF_LEN];
				int num_char_read;
				num_char_read = read(cur_checking->cfd, &read_buf, READ_SIZE);
				if (num_char_read == -1) {
					fprintf(stderr, "read failed from client");
				} else {
					printf("Message from client with cfd %d:, %s", cur_checking->cfd, read_buf);
				}
			}
		}



	}

	unlink(MY_SOCK_PATH);

	return 0;
}
