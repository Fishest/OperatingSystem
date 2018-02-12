#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//Define a system call implementation that takes no arguments
SYSCALL_DEFINE0( studio3_void ){
//
//  // print out a simple message indicating the function was called, and return SUCCESS
    printk("Someone invoked the sys_noargs system call");
      return 0;  
}
