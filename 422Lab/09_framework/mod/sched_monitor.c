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
#include <linux/timekeeping.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

/* DO NOT MODIFY THIS CHECK. If you see the message in the error line below
 * when compliling your module, that means your kernel has not been configured
 * correctly. Complete exercises 1-3 of the lab before continuing
 */
#ifdef CONFIG_PREEMPT_NOTIFIERS
#include <linux/preempt.h>
#else
#error "Your kernel must be built with CONFIG_PREEMPT_NOTIFIERS enabled"
#endif

#include <sched_monitor.h>

#define MAX_PREEMPTS 32768

/* Information tracked on each preemption. */
struct preemption_entry
{
    /* TODO: populate this */
    //struct preemption_tracker tracker;
    struct preemption_info * pit;
    struct list_head list_node;
};

struct preemption_tracker
{
    /* notifier to register us with the kernel's callback mechanisms */
    struct preempt_notifier notifier;
    bool enabled;
    bool first_sched_out;
    /* TODO: anything else you need */
    struct preemption_entry * head;
    struct preemption_entry * current_entry;
    struct spinlock list_lock;
    unsigned long lock_macro;
};
/* Get the current time, in nanoseconds. Should be consistent across cores.
 * You are encouraged to look at the options in:
 *      include/linux/timekeeping.h
 */
static inline unsigned long long
get_current_time(void)
{
    /* TODO: implement */
    struct timespec tc;
    getnstimeofday(&tc);
    return ((unsigned long long)tc.tv_sec) * 1000000000 + (unsigned long long)tc.tv_nsec;
}

/* 
 * Utilities to save/retrieve a tracking structure for a process
 * based on the kernel's file pointer.
 *
 * DO NOT MODIFY THE FOLLOWING 2 FUNCTIONS
 */
static inline void
save_tracker_of_process(struct file               * file,
                        struct preemption_tracker * tracker)
{
    file->private_data = tracker;
}

static inline struct preemption_tracker *
retrieve_tracker_of_process(struct file * file)
{
    return (struct preemption_tracker *)file->private_data;
}

/*
 * Utility to retrieve a tracking structure based on the 
 * preemption_notifier structure.
 *
 * DO NOT MODIFY THIS FUNCTION
 */
static inline struct preemption_tracker *
retrieve_tracker_of_notifier(struct preempt_notifier * notifier)
{
    return container_of(notifier, struct preemption_tracker, notifier);
}

/* 
 * Callbacks for preemption notifications.
 *
 * monitor_sched_in and monitor_sched_out are called when the process that
 * registered a preemption notifier is either scheduled onto or off of a
 * processor.
 *
 * You will use these functions to create a linked_list of 'preemption_entry's
 * With this list, you must be able to represent information such as:
 *      (i)   the amount of time a process executed before being preempted
 *      (ii)  the amount of time a process was scheduled out before being
 *            scheduled back on 
 *      (iii) the cpus the process executes on each time it is scheduled
 *      (iv)  the number of migrations experienced by the process
 *      (v)   the names of the processes that preempt this process
 *
 * The data structure 'preemption_tracker' will be necessary to track some of
 * this information.  The variable 'pn' provides unique state that persists
 * across invocations of monitor_sched_in and monitor_sched_out. Add new fields
 * to it as needed to provide the information above.
 */

static void
monitor_sched_in(struct preempt_notifier * pn,
                 int                       cpu)
{
    struct preemption_tracker * tracker = retrieve_tracker_of_notifier(pn);

    printk(KERN_INFO "sched_in for process %s\n", current->comm);
    /* TODO: record information as needed */
    struct preemption_entry * new_entry;
    struct preemption_info * new_info;
    new_entry = (struct preemption_entry *)kmalloc(sizeof(struct preemption_entry), GFP_KERNEL);
    new_info = (struct preemption_info *)kmalloc(sizeof(struct preemption_info), GFP_KERNEL);
    new_info->start_time = get_current_time();
    new_info->end_time = 0;
    new_info->duration = 0;
    new_info->core_num = cpu;
    new_entry->pit = new_info;
    list_add_tail(&new_entry->list_node, &tracker->head->list_node);
    tracker->current_entry = new_entry;
 }

static void
monitor_sched_out(struct preempt_notifier * pn,
                  struct task_struct      * next)
{
    struct preemption_tracker * tracker = retrieve_tracker_of_notifier(pn);
    if (tracker->first_sched_out) {
        //ignore
        tracker->first_sched_out = 0;
        return;
    }
    printk(KERN_INFO "sched_out for process %s\n", current->comm);
    /* TODO: record information as needed */
    struct preemption_info * info;
    info = tracker->current_entry->pit;
    info->end_time = get_current_time();
    info->duration = info->end_time - info->start_time;
    strcpy(&info->preempt_process_name, next->comm);
}

static struct preempt_ops
notifier_ops = 
{
    .sched_in  = monitor_sched_in,
    .sched_out = monitor_sched_out
};

/*** END preemption notifier ***/


/*
 * Device I/O callbacks for user<->kernel communication 
 */
/*
 * This function is invoked when a user opens the file /dev/sched_monitor.
 * You must update it to allocate the 'tracker' variable and initialize 
 * any fields that you add to the struct
 */
static int
sched_monitor_open(struct inode * inode,
                   struct file  * file)
{
    struct preemption_tracker * tracker;

    //printk(KERN_DEBUG "Process %d (%s) tried to open " DEV_NAME ", but this is disabled\n",
    //    current->pid, current->comm);

    //return -ENODEV;
    tracker = (struct preemption_tracker *)kmalloc(sizeof(struct preemption_tracker), GFP_KERNEL);
    tracker->enabled = false;
    tracker->first_sched_out = true;
    /* TODO: allocate a preemption_tracker for this process, other initialization
     * as needed. The rest of this function will trigger a kernel oops until
     * you properly allocate the variable 'tracker'
     */
    struct preemption_entry * dummy;
    dummy = (struct preemption_entry *)kmalloc(sizeof(struct preemption_entry), GFP_KERNEL);
    struct preemption_info * info, *dummy_info;
    dummy_info = (struct preemption_info *)kmalloc(sizeof(struct preemption_info), GFP_KERNEL);
    spin_lock_init(&tracker->list_lock);
    dummy->pit = dummy_info;
    INIT_LIST_HEAD(&dummy->list_node);
    tracker->head = dummy;
    tracker->current_entry = dummy;
    //mutex_init(&tracker->list_lock);
#if 1
    /* setup tracker */
    /* initialize preempt notifier object */
    preempt_notifier_init(&(tracker->notifier), &notifier_ops);
    /* Save tracker so that we can access it on other file operations from this process */
    save_tracker_of_process(file, tracker);
    printk(KERN_DEBUG "Tracker saved\n");
    printk(KERN_DEBUG "At time %llu\n", get_current_time());
    return 0;
#endif
}

/* This function is invoked when a user closes /dev/sched_monitor.
 * You must update is to free the 'tracker' variable allocated in the 
 * sched_monitor_open callback, as well as free any other dynamically 
 * allocated data structures you may have allocated (such as linked lists)
 */
static int
sched_monitor_flush(struct file * file,
                    fl_owner_t    owner)
{
    struct preemption_tracker * tracker = retrieve_tracker_of_process(file);

    printk(KERN_DEBUG "Process %d (%s) closed " DEV_NAME "\n",
        current->pid, current->comm);
    printk(KERN_DEBUG "At time %llu\n", get_current_time());
    /* Unregister notifier */
    tracker->current_entry->pit->end_time = get_current_time();
    tracker->current_entry->pit->duration = tracker->current_entry->pit->end_time - tracker->current_entry->pit->start_time;
    char message[14] = "monitor_flush";
    strcpy(&tracker->current_entry->pit->preempt_process_name, message);
    if (tracker->enabled) {
        /* TODO: unregister notifier */
        preempt_notifier_unregister(&(tracker->notifier));
	tracker->enabled = false;
    }
    spin_lock_irqsave(&tracker->list_lock, tracker->lock_macro);
    /* TODO: other teardown, freeing, etc. */
    struct preemption_entry * cur, *next;
    if (!list_empty(&tracker->head->list_node)) {

    //printk(KERN_DEBUG "The head has %llu start time and %llu end time, and kicked by %s\n", tracker->head->pit->start_time, tracker->head->pit->end_time, tracker->head->pit->preempt_process_name);
    list_for_each_entry_safe(cur, next, &tracker->head->list_node, list_node) {
    	printk(KERN_DEBUG "Freeing preemption entry with %llu start time, %llu end time, kicked by %s\n", cur->pit->start_time, cur->pit->end_time, cur->pit->preempt_process_name);
	list_del(&cur->list_node);
        //kfree(cur->pit);
	kfree(cur);
    }
    //kfree(tracker->head);
    }
    spin_unlock_irqrestore(&tracker->list_lock, tracker->lock_macro);
    return 0;
}

/* 
 * Enable/disable preemption tracking for the process that opened this file.
 * Do so by registering/unregistering preemption notifiers.
 */
static long
sched_monitor_ioctl(struct file * file,
                    unsigned int  cmd,
                    unsigned long arg)
{
    struct preemption_tracker * tracker = retrieve_tracker_of_process(file);

    switch (cmd) {
        case ENABLE_TRACKING:
            if (tracker->enabled) {
                printk(KERN_ERR "Tracking already enabled for process %d (%s)\n",
                    current->pid, current->comm);
                return 0;
            }

            /* TODO: register notifier, set enabled to true, and remove the error return */
            tracker->enabled = true;
	    preempt_notifier_register(&(tracker->notifier));
            //printk(KERN_DEBUG "Process %d (%s) trying to enable preemption tracking via " DEV_NAME ". Not implemented\n",
            //    current->pid, current->comm);

            //return -ENOIOCTLCMD;

            break;


        case DISABLE_TRACKING:
            if (!tracker->enabled) {
                printk(KERN_ERR "Tracking not enabled for process %d (%s)\n",
                    current->pid, current->comm);
                return 0;
            }

            /* TODO: unregister notifier, set enabled to true, and remove the error return */
            tracker->enabled = false;
	    preempt_notifier_unregister(&(tracker->notifier));
            //printk(KERN_DEBUG "Process %d (%s) trying to disable preemption tracking via " DEV_NAME ". Not implemented\n",
            //    current->pid, current->comm);

            //return -ENOIOCTLCMD;

            break;

        default:
            printk(KERN_ERR "No such ioctl (%d) for " DEV_NAME "\n", cmd);
            return -ENOIOCTLCMD;
    }

    return 0;
}

/* User read /dev/sched_monitor
 *
 * In this function, you will copy an entry from the list of preemptions
 * experienced by the process to user-space.
 */
static ssize_t
sched_monitor_read(struct file * file,
                   char __user * buffer,
                   size_t        length,
                   loff_t      * offset)
{
    struct preemption_tracker * tracker = retrieve_tracker_of_process(file);
    unsigned long flags;

    /* TODO: 
     * (1) make sure length is valid. It must be an even multiuple of the size of preemption_info_t.
     *     i.e., if the value is the size of the structure times 2, the user is requesting the first
     *     2 entries in the list
     * (2) lock your linked list
     * (3) retrieve the head of the linked list, if it is not empty
     * (4) copy the associated contents to user space
     * (5) remove the head of the linked list and free its dynamically allocated
     *     storage.
     * (6) repeat for each entry
     * (7) unlock the list
     */
    if (length % sizeof(struct preemption_info) != 0) {
	printk(KERN_ERR "Length not multiple of info size");
    }
    int multiple;
    multiple = length / sizeof(struct preemption_info);
    
     
    printk(KERN_DEBUG "Process %d (%s) read " DEV_NAME "\n",
        current->pid, current->comm);
    if (list_empty(&tracker->head->list_node)) {
        printk(KERN_DEBUG "List empty\n");
        return 0;
    }
    spin_lock_irqsave(&tracker->list_lock, tracker->lock_macro);
  
    struct preemption_entry *cur, *n;
    struct preemption_info *pit;
    int count = 0;
    unsigned long ret;
    list_for_each_entry_safe(cur, n, &tracker->head->list_node, list_node) {
        if (count == multiple) {
            break;
        }
        count ++;
        pit = cur->pit;
        printk(KERN_DEBUG "Trying to copy over %u bytes\n", sizeof(struct preemption_info));
        printk(KERN_DEBUG "Start time : %llu, end time: %llu, kicked by %s\n", pit->start_time, pit->end_time, pit->preempt_process_name); 
        ret = copy_to_user(buffer, pit, sizeof(struct preemption_info));
        if (ret != 0) {
           printk(KERN_DEBUG "Failed to copy to user buffer %lu bytes\n", ret);
           break;
        }
        buffer += sizeof(struct preemption_info);
        list_del(&cur->list_node);
        kfree(cur->pit);
        kfree(cur);
     }
     printk(KERN_DEBUG "Copy finished");
    /* Use these functions to lock/unlock our list to prevent concurrent writes
     * by the preempt notifier callouts
     *
     *  spin_lock_irqsave(<pointer to spinlock_t variable>, flags);
     *  spin_unlock_irqrestore(<pointer to spinlock_t variable>, flags);
     *
     *  Before the spinlock can be used, it will have to be initialized via:
     *  spin_lock_init(<pointer to spinlock_t variable>);
     */
    spin_unlock_irqrestore(&tracker->list_lock, tracker->lock_macro); 
    ssize_t ret_size = sizeof(struct preemption_info) * count - ret;
    printk(KERN_DEBUG "copied over %u bytes in total with ret %d and %d entries\n", sizeof(struct preemption_info), ret, count);
    return ret_size;
}

static struct file_operations
dev_ops = 
{
    .owner = THIS_MODULE,
    .open = sched_monitor_open,
    .flush = sched_monitor_flush,
    .unlocked_ioctl = sched_monitor_ioctl,
    .compat_ioctl = sched_monitor_ioctl,
    .read = sched_monitor_read,
};

static struct miscdevice
dev_handle = 
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = SCHED_MONITOR_MODULE_NAME,
    .fops = &dev_ops,
};
/*** END device I/O **/



/*** Kernel module initialization and teardown ***/
static int
sched_monitor_init(void)
{
    int status;

    /* Create a character device to communicate with user-space via file I/O operations */
    status = misc_register(&dev_handle);
    if (status != 0) {
        printk(KERN_ERR "Failed to register misc. device for module\n");
        return status;
    }

    /* Enable preempt notifiers globally */
    preempt_notifier_inc();

    printk(KERN_INFO "Loaded sched_monitor module. HZ=%d\n", HZ);

    return 0;
}

static void 
sched_monitor_exit(void)
{
    /* Disable preempt notifier globally */
    preempt_notifier_dec();

    /* Deregister our device file */
    misc_deregister(&dev_handle);

    printk(KERN_INFO "Unloaded sched_monitor module\n");
}

module_init(sched_monitor_init);
module_exit(sched_monitor_exit);
/*** End kernel module initialization and teardown ***/


/* Misc module info */
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Brian Kocoloski (change to your name(s))");
MODULE_DESCRIPTION ("CSE 422S Lab 1");
