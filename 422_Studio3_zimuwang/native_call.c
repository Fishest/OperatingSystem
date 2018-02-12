#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/syscall.h>
#define _GNU_SOURCE

int main() {
        uid_t uid = syscall(__NR_getuid);
        printf("User ID %d\n", uid);
        int result = syscall(__NR_setuid, 0);
        if (result != 0) {
                printf("Error, setuid failed, %s\n", strerror(errno));
        }
        uid = getuid();
        printf("User ID after %d\n", uid);

}

