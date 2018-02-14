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

struct node{
	FILE* file;
	struct node *next;
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

	FILE* f_in, f_out;
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
  n = fscanf(f_in, "%[^\n]\n",&output_filename);
  f_out = fopen(output_filename, "w");
  if (f_out == NULL) {
    fprintf(stderr, "Cannot open file for output %s\n", output_filename)
  }
  root.file = f_out;

  // Read in all the file names
	while(1){
		n = fscanf(spec,"%[^\n]\n",&line);
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
		current_node->next=new_node;
		current_node=current_node->next;
		count++;
	}

	current_node=root->next;
	while(current_node->next!=NULL){
		current_node=current_node->next;
		n=fscanf(current_node->file,"%[^\n]\n",&line);
		if(n==-1){
			handle_error("read");
		}
		printf("%s\n",&line);
	}

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

	while(1){
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(sfd, &readfds);

		tv.tv_sec=TIMEOUT;
		tv.tv_usec=0;

		if(sfd>STDIN_FILENO){
			biggest_fd=sfd;
		}else{
			biggest_fd=STDIN_FILENO;
		}
		select_ret=select(biggest_fd+1,&readfds,NULL,NULL,&tv);
		if(select_ret==-1){
			handle_error("select");
		}else if(!select_ret){
			printf("%d seconds elapsed.\n",TIMEOUT);
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
		if(FD_ISSET(sfd, &readfds)){
			ssize_t write_size;
			peer_addr_size = sizeof(struct sockaddr_in);
			char write_buf[BUF_LEN];
			char message[] = "hostname: mhost";
			strncpy(write_buf,message,sizeof(message));
			fprintf(stderr, "Before accept\n");
			cfd = accept(sfd, (struct sockaddr *) &peer_addr,
        &peer_addr_size);
			fprintf(stderr, "After accept\n");
			if (cfd == -1)
				handle_error("accept");
			fprintf(stdout, "things to write %s\n",message);
			printf("message size: %d\n",strlen(message));
			write_size = write(cfd, &message, strlen(message));
			fprintf(stdout, "write size: %d\n",write_size);
			if (write_size == -1)
				handle_error("write");
		}
	}

	unlink(MY_SOCK_PATH);

  int current;
  select_ret=select(biggest_fd+1,&readfds,NULL,NULL,&tv);






	return 0;
}
