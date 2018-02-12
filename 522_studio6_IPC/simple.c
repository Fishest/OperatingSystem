#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define NOT_WAITING      0
#define WAITING_FOR_SIG1 1
#define WAITING_FOR_SIG2 2

#define PIPE_BUF 80

#define OWNER_READ_WRITE_EXEC 0700
#define MY_FIFO "myfifo"
#define FIFO_BUF 80

#define LOCAL_SOCK_PATH "/home/pi/studio6/socket"
#define LISTEN_BACKLOG 50

#define NET_SOCK_PATH "127.0.0.1"
#define PORT 1026

#define SOCK_BUF 80
#define READ_SIZE 10
#define WRITE_SIZE 10

const char* pmessage1 = "phandler1\n";
const char* pmessage2 = "phandler2\n";
const char* cmessage = "chandler\n";

atomic_int pflag;
int pcounter;
int ccounter;

void phandler1(int signum){
	//write(0, pmessage1, strlen(pmessage1)+1);
	pflag = WAITING_FOR_SIG2;
}

void phandler2(int signum){
	//write(0, pmessage2, strlen(pmessage2)+1);
	pflag = NOT_WAITING;	
}

void chandler(int signum){
	/*write(0, cmessage, strlen(cmessage)+1);*/
	ccounter++;
}

int main(int argc,char* argv[])
{
	pid_t cpid;
	int counter;
	char* mechanism;
	char buf[PIPE_BUF];
	int pipefd[2];
	int childfd, parentfd;
	int fifo_read_buf;
	int csfd, ccfd, ssfd, scfd;
	struct sockaddr_un my_lcaddr, peer_lcaddr, my_lsaddr, peer_lsaddr;
	struct sockaddr_in my_ncaddr, peer_ncaddr, my_nsaddr, peer_nsaddr;
	socklen_t peer_addr_size;
	if(argc != 3){
		printf("Usage: %s counter mechanism\n", argv[0]);
		return 1;
	}
	counter = atoi(argv[1]);
	mechanism = argv[2];
	/*if (strcmp(mechanism, "signals")==0
		||strcmp(mechanism,"pipe")==0
		||strcmp(mechanism,"FIFO")==0
		||strcmp(mechanism,"lsock")==0){*/

	struct sigaction psigact1;
	psigact1.sa_handler = phandler1;
	psigact1.sa_flags = SA_RESTART;
	if (sigaction(SIGUSR1, &psigact1, NULL)==-1){
		printf("Parent's first sigaction: %s\n",
			strerror(errno));
		return 1;
	}
	struct sigaction psigact2;
  psigact2.sa_handler = phandler2;
  psigact2.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR2, &psigact2, NULL)==-1){
		printf("Parent's second sigaction: %s\n",
			strerror(errno));
		return 1;
	}
	/*}*/
	if (strcmp(mechanism, "pipe")==0){
		if (pipe(pipefd)==-1){
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}
	if(strcmp(mechanism,"FIFO")==0){
		if(mkfifo(MY_FIFO, OWNER_READ_WRITE_EXEC)==-1){
			perror("mkfifo");
			exit(EXIT_FAILURE);
		}
	}
	
	pflag = WAITING_FOR_SIG1;
	cpid = fork();
	if(cpid==-1){
		printf("Fork: %s\n", strerror(errno));
	}
	else if(cpid==0){
		printf("Child, %d, %s\n", counter, mechanism);
		fflush(stdout);
		if (strcmp(mechanism, "signals")==0){
			struct sigaction csigact;
			csigact.sa_handler = chandler;
			csigact.sa_flags = SA_RESTART;
			if (sigaction(SIGUSR2, &csigact, NULL)==-1){
				printf("Child's sigaction: %s\n",
					strerror(errno));
				return 1;
			}
		}	
		if(strcmp(mechanism,"lsock")==0){
			ssfd=socket(AF_UNIX,SOCK_STREAM,0);
			if(ssfd==-1){
				perror("socket");
				exit(EXIT_FAILURE);
			}
			memset(&my_lsaddr,0,sizeof(struct sockaddr_un));
			my_lsaddr.sun_family=AF_UNIX;
			strncpy(my_lsaddr.sun_path,LOCAL_SOCK_PATH,
				sizeof(my_lsaddr.sun_path)-1);
			if(bind(ssfd,(struct sockaddr *)&my_lsaddr,
				sizeof(struct sockaddr_un))==-1){

				perror("bind");
				exit(EXIT_FAILURE);
			}
			if(listen(ssfd,LISTEN_BACKLOG)==-1){
				perror("listen");
				exit(EXIT_FAILURE);
			}	
		}
		if(strcmp(mechanism,"socket")==0){
			ssfd=socket(AF_INET,SOCK_STREAM,0);
			if(ssfd==-1){
				perror("socket");
				exit(EXIT_FAILURE);
			}
			memset(&my_nsaddr,0,sizeof(struct sockaddr_in));
			my_nsaddr.sin_family=AF_INET;
			my_nsaddr.sin_port=htons(PORT);
			if(inet_aton(NET_SOCK_PATH,&(my_nsaddr.sin_addr))==0){
					fprintf(stderr,"inet_aton");
			}
			if(bind(ssfd,(struct sockaddr *)&my_nsaddr,
				sizeof(struct sockaddr_in))==-1){

				perror("bind");
				exit(EXIT_FAILURE);
			}
			if(listen(ssfd,LISTEN_BACKLOG)==-1){
				perror("listen");
				exit(EXIT_FAILURE);
			}	
		}
		kill(getppid(), SIGUSR1);
		if(strcmp(mechanism, "signals")==0){
			while (ccounter < counter){
			}
		}
		if(strcmp(mechanism, "pipe")==0){
			int n;
			close(pipefd[1]);//close writer
			while(ccounter<counter){
				n = read(pipefd[0],&buf
					,PIPE_BUF+1);
				if (n<0){
					perror("read");
					exit(EXIT_FAILURE);
				}
				ccounter++;
			}
		}
		if(strcmp(mechanism,"FIFO")==0){
			char buf[FIFO_BUF];
			childfd=open(MY_FIFO,O_RDONLY);
			if (childfd==-1){
				perror("child open");
				exit(EXIT_FAILURE);
			}
			while(ccounter<counter){
				if (read(childfd,buf,FIFO_BUF)!=0){
					ccounter++;
				}
			}
			close(childfd);
		}
		if(strcmp(mechanism,"lsock")==0){
			scfd=accept(ssfd,(struct sockaddr *) &peer_lsaddr,
				&peer_addr_size);
			if(scfd==-1){
				perror("server accept");
				exit(EXIT_FAILURE);
			}

			ssize_t read_size;
			char read_buf[SOCK_BUF];
			while(ccounter<counter){
			  read_size=read(scfd, &read_buf, READ_SIZE);
			  if(read_size==-1){
			 	  perror("read");
				  exit(EXIT_FAILURE);
			  }
			  if(read_size==0){}
				ccounter++;
			}
			unlink(LOCAL_SOCK_PATH);
		}
		if(strcmp(mechanism,"socket")==0){
			scfd=accept(ssfd,(struct sockaddr *) &peer_nsaddr,
				&peer_addr_size);
			if(scfd==-1){
				perror("server accept");
				exit(EXIT_FAILURE);
			}

			ssize_t read_size;
			char read_buf[SOCK_BUF];
			while(ccounter<counter){
			  read_size=read(scfd, &read_buf, READ_SIZE);
			  if(read_size==-1){
			 	  perror("read");
				  exit(EXIT_FAILURE);
			  }
			  if(read_size==0){}
				ccounter++;
			}
			unlink(NET_SOCK_PATH);
		}

		kill(getppid(), SIGUSR2);
	}else{
		printf("Parent, %d, %s\n", counter, mechanism);
		fflush(stdout);
		int i;
		for (i=0;i<PIPE_BUF;i++){
			buf[i]=(char)0;
		}
		struct timespec st, et;
		while(pflag == WAITING_FOR_SIG1){
		}
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &st)==-1)
			printf("Get start time: %s\n",
				strerror(errno));
		if (strcmp(mechanism, "signals")==0){
			while(pflag==WAITING_FOR_SIG2){
				kill(cpid, SIGUSR2);
				pcounter++;
			}
		}
		if(strcmp(mechanism,"pipe")==0){
			close(pipefd[0]);//close reader
			while(pflag==WAITING_FOR_SIG2){
				write(pipefd[1],&buf,PIPE_BUF+1);
				pcounter++;
			}
			close(pipefd[1]);//close writer
		}
		if(strcmp(mechanism, "FIFO")==0){
			char buf[FIFO_BUF];
			strncpy(buf,"somethingtowrite",sizeof(buf)-1);
			parentfd=open(MY_FIFO,O_WRONLY);
			if (parentfd==-1){
				perror("parent open");
				exit(EXIT_FAILURE);
			}
			while(pflag==WAITING_FOR_SIG2){
				write(parentfd,buf,strlen(buf)+1);
				pcounter++;
			}
			close(parentfd);
		}
		if(strcmp(mechanism,"lsock")==0){
			csfd=socket(AF_UNIX,SOCK_STREAM,0);
			if(csfd==-1){
				perror("socket");
				exit(EXIT_FAILURE);
			}
			memset(&my_lcaddr,0,sizeof(struct sockaddr_un));
			my_lcaddr.sun_family=AF_UNIX;
			strncpy(my_lcaddr.sun_path,LOCAL_SOCK_PATH,
				sizeof(my_lcaddr.sun_path)-1);
			while(connect(csfd,(struct sockaddr*) &my_lcaddr,
				sizeof(struct sockaddr_un))==-1){
			}
			ssize_t write_size;
			char write_buf[SOCK_BUF]="tencharac";
			while(pflag==WAITING_FOR_SIG2){
				write_size=write(csfd,&write_buf,WRITE_SIZE);
				if(write_size==-1){
					perror("write");
					exit(EXIT_FAILURE);
				}
				pcounter++;
			}
		}
		if(strcmp(mechanism,"socket")==0){
			csfd=socket(AF_INET,SOCK_STREAM,0);
			if(csfd==-1){
				perror("socket");
				exit(EXIT_FAILURE);
			}
			memset(&my_ncaddr,0,sizeof(struct sockaddr_in));
			my_ncaddr.sin_family=AF_INET;
			my_ncaddr.sin_port=htons(PORT);
			if(inet_aton(NET_SOCK_PATH,&(my_ncaddr.sin_addr))==0){
					fprintf(stderr,"inet_aton");
			}
			
			while(connect(csfd,(struct sockaddr*) &my_ncaddr,
				sizeof(struct sockaddr_in))==-1){
			}
			ssize_t write_size;
			char write_buf[SOCK_BUF]="tencharac";
			while(pflag==WAITING_FOR_SIG2){
				write_size=write(csfd,&write_buf,WRITE_SIZE);
				if(write_size==-1){
					perror("write");
					exit(EXIT_FAILURE);
				}
				pcounter++;
			}
		}
		while(pflag==WAITING_FOR_SIG2){
		}		
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &et)==-1)
			printf("Get end time: %s\n",
				strerror(errno));
		wait(NULL);
		fprintf(stdout,"The parent process sent %d\n", pcounter);
		fprintf(stdout,"signals to the child process.\n");
		fprintf(stdout,"This process took %ld nanoseconds.\n", 
			et.tv_nsec-st.tv_nsec);
		fflush(stdout);
	}
	
	return 0;
}
