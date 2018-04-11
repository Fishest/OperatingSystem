#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/mm.h>

#include <asm/uaccess.h>

static struct task_struct * kthread = NULL;

static int
thread_fn(void * data)
{
    printk(KERN_INFO "Kernel thread started.\n");
    printk(KERN_INFO "fs: %lu, files: %lu, nsproxy: %lu", current->fs, current->files, current->nsproxy);


    while (!kthread_should_stop()) {
        schedule();
    }

    return 0;
}

static int
kernel_memory_init(void)
{
    printk(KERN_INFO "Loaded kernel_memory module\n");

    kthread = kthread_create(thread_fn, NULL, "studio14");

    wake_up_process(kthread);

    return 0;
}

static void
kernel_memory_exit(void)
{
    kthread_stop(kthread);

    printk(KERN_INFO "Unloaded kernel_memory module\n");
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");
