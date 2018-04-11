#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SUCCESS 0
#define SPIN 1
#define BUF_LEN 1024
#define MAX_NUM_FILES_WATCHED 10
#define MAX_PATH_LEN 100

int main(int argc, char* args[]){
  if (argc!=2){
    printf("Usage: %s [path to watch]\n", args[0]);
    exit(EXIT_FAILURE);
  }
  char* path=args[1];
  /* Step 2 */
  int fd;
  fd = inotify_init1(0);
  if (fd==-1){
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }else{
    printf("Successfully initialize inotify.\n");
    printf("File descriptor number is %d.\n", fd);
  }

  /* Step 3*/
  int wd;
  wd=inotify_add_watch(fd,path, IN_ALL_EVENTS);
  if(wd==-1){
    perror("inotify_add_watch_to_all_events");
    exit(EXIT_FAILURE);
  }else{
    printf("Add watch successful.\n");
    printf("File descriptor number is %d.\n",wd);
  }

  /* Step 5*/
  /*
  int wd2;
  wd2=inotify_add_watch(fd,path,IN_MOVE);
  if(wd2==-1){
    perror("inotify_add_watch_to_in_move");
    exit(EXIT_FAILURE);
  }else{
    printf("Add watch successful.\n");
    printf("File descriptor number is %d.\n",wd2);
  }
  */

  /* Step 6*/
  int* wds;
  wds=(int *)malloc(sizeof(int)*MAX_NUM_FILES_WATCHED);
  int current_wds_size=1;
  wds[0]=wd;

  /* Step 4*/
  while(SPIN){
     char buf[BUF_LEN] __attribute__(
       (aligned(__alignof__(struct inotify_event))));
     ssize_t len,i=0;

     /* read BUF_LEN bytes' worth of events*/
     len=read(fd,buf,BUF_LEN);

     /* loop over every read event until none remain */
     while(i<len){
       struct inotify_event *event=
         (struct inotify_event *) &buf[i];
       printf("wd=%d mask=%d cookie=%d len=%d dir=%s\n",
         event->wd, event->mask,
         event->cookie, event->len,
         (event->mask & IN_ISDIR) ? "yes":"no");

       /* if there is a name, print if*/
       if(event->len){
         printf("name=%s\n",event->name);
       }

       /* update the index to the start of the next event*/
       i+=sizeof(struct inotify_event)+event->len;

       /*Step 6*/
       /* check if new file system object has been created 
        * in the original directory passed in as an argument    
        */
       if(event->wd==wds[0]&&(event->mask & IN_CREATE)){
         if(current_wds_size<MAX_NUM_FILES_WATCHED){
           int new_wd;
           char* new_file_path=(char*)malloc(MAX_PATH_LEN+1);
           strncpy(new_file_path,path,strlen(path));
           strncat(new_file_path,event->name,strlen(event->name));
           new_wd=inotify_add_watch(fd,new_file_path,IN_ALL_EVENTS);
           if(new_wd==-1){
             perror("fail to add new wd");
             exit(EXIT_FAILURE);
           }
           wds[current_wds_size++]=new_wd;
         }
       }
     }
  }
  return SUCCESS;
}