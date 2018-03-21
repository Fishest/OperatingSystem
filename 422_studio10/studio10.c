#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>

volatile int shared_data = 0;

#define iter 1000000

static struct task_struct* threads[4];

static int thread_work(void * data) {
	int i;
		for (i = 0; i < iter; i++) {
			shared_data++;
		}
	while (!kthread_should_stop()) {
		schedule();
	}
	//printk(KERN_DEBUG "THread finished");
}

static int spawn_init(void) {
	int index;
	for (index = 0; index < 4; index ++) {
		printk(KERN_DEBUG "Creating thread # %d", index);
		threads[index] = kthread_create(&thread_work, NULL, "thread");
		if (threads[index] == NULL) {
			printk(KERN_DEBUG "Thread creation failed in # %d", index);
		}
		kthread_bind(threads[index], index);
		wake_up_process(threads[index]);
	}
	return 0;
}

static int spawn_exit(void) {
	int index;
	int ret;
	printk(KERN_DEBUG "Shared_data %d", shared_data);
        for (index = 0; index < 4; index ++) {
        	printk(KERN_DEBUG "Stopping thread # %d", index);
		if (threads[index] != NULL) {
			ret = kthread_stop(threads[index]);
			if (ret != 0) {
				printk(KERN_DEBUG "Thread stopping failed");
			}	
		}	
	}
}



module_init(spawn_init);
module_exit(spawn_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Zimu Wang");
MODULE_DESCRIPTION ("Studio 10");
