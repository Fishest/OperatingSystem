#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int main() {
        uid_t uid = getuid();   
        printf("User ID %d\n", uid);
        int result = setuid(0);
        if (result != 0) {
                printf("Error, setuid failed, %s\n", strerror(errno));
        }
        uid = getuid(); 
        printf("User ID after %d\n", uid); 
    
}
