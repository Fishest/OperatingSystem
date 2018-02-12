#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//Define a system call implementation that takes no arguments
SYSCALL_DEFINE1( studio3_int, int, cmd ){
//
//  // print out a simple message indicating the function was called, and return SUCCESS
    printk("Someone invoked the sys_studio3_int system call with integer %d", cmd);
    return 0;  
}
