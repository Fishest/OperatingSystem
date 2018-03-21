#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sched_monitor.h>

int
main(int     argc,
     char ** argv)
{
    int fd, status;

    fd = open(DEV_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Could not open %s: %s\n", DEV_NAME, strerror(errno));
        return -1;
    }

    status = enable_preemption_tracking(fd);
    if (status < 0) {
        fprintf(stderr, "Could not enable preemption tracking on %s: %s\n", DEV_NAME, strerror(errno));
        close(fd);
        return -1;
    }

    int ctr, i;
    ctr = 1;
    for (i = 0; i < 10000000; i++) {
       ctr *= 2;
    }

    char buff[100];
    int ret = 1;

        ret = read(fd, buff, sizeof(struct preemption_info));
        buff[ret] = '\0';
        printf("Read %d bytes from preemption_info with actual size %u", ret, sizeof(struct preemption_info));
        write(0, buff, ret);
        struct preemption_info * pit = (struct preemption_info *)buff;
        printf("Start time : %llu, endtime : %llu, kicked by %s\n", pit->start_time, pit->end_time, pit->preempt_process_name);
    ret = read(fd, buff, sizeof(struct preemption_info));
        buff[ret] = '\0';
        printf("Read %d bytes from preemption_info with actual size %u", ret, sizeof(struct preemption_info));
        write(0, buff, ret);
    ret = read(fd, buff, sizeof(struct preemption_info));
        buff[ret] = '\0';
        printf("Read %d bytes from preemption_info with actual size %u", ret, sizeof(struct preemption_info));
        write(0, buff, ret);
    status = disable_preemption_tracking(fd);
    if (status < 0) {
        fprintf(stderr, "Could not disable preemption tracking on %s: %s\n", DEV_NAME, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);

    printf("Monitor ran to completion!\n");

    return 0;
}
