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
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/mm.h>
#include <linux/spinlock.h>

#define TIMER_COUNT 3

static unsigned long num_threads = 1;
static unsigned long upper_bound = 10;
module_param(num_threads, ulong, 0644);
module_param(upper_bound, ulong, 0644);

static unsigned long * counter;
static volatile int * arr;
static volatile unsigned long curr;

struct spinlock cur_lock, array_lock;
unsigned long cur_lock_macro, array_lock_macro;

static atomic_t num_threads_count = ATOMIC_INIT(0);
static atomic_t num_threads_count1 = ATOMIC_INIT(0);

static ktime_t times[TIMER_COUNT];

static void
synchronize_start_barrier(void)
{
								int is_last = 1;
								if (atomic_read(&num_threads_count) == 1) {
																is_last = 0;
								}
								atomic_sub(1, &num_threads_count);

								while (atomic_read(&num_threads_count) > 0) {
																schedule();
								}
								if (is_last == 0) {
																times[1] = ktime_get();
								}
}

static void
synchronize_end_barrier(void)
{
								int is_last = 1;
								if (atomic_read(&num_threads_count1) == 1) {
																is_last = 0;
								}
								atomic_sub(1, &num_threads_count1);

								while (atomic_read(&num_threads_count1) > 0) {
																schedule();
								}
								if (is_last == 0) {
																times[2] = ktime_get();
								}
}

static void
mark_nonprimes(int *ptr)
{
								while (1) {
																spin_lock_irqsave(&cur_lock, cur_lock_macro);
																while (curr <= upper_bound && arr[curr] == 0) {
																								curr++;
																}
																if (curr > upper_bound) {
																								spin_unlock_irqrestore(&cur_lock, cur_lock_macro);
																								return;
																}
																unsigned long prime = curr;
																curr++;
																spin_unlock_irqrestore(&cur_lock, cur_lock_macro);
																printk(KERN_INFO "Working on %lu\n", curr);
																unsigned long set_zero = prime;
																//spin_lock_irqsave(&array_lock, array_lock_macro);
																while (set_zero + prime <= upper_bound) {
																								set_zero += prime;
																								spin_lock_irqsave(&array_lock, array_lock_macro);
																								arr[set_zero] = 0;
																								(*ptr)++;
																								spin_unlock_irqrestore(&array_lock, array_lock_macro);
																}
																//spin_unlock_irqrestore(&array_lock, array_lock_macro);

								}
}

static int
thread_fn(void *void_ptr)
{
								int *counter_ptr = (int *)void_ptr;

								synchronize_start_barrier();
								printk(KERN_INFO "Doing work\n");
								mark_nonprimes(counter_ptr);
								printk(KERN_INFO "Work finished\n");
								synchronize_end_barrier();
}

/* init function - logs that initialization happened, returns success */
static int
simple_init(void)
{
								printk(KERN_ALERT "simple module initialized\n");

								times[0] = ktime_get();

								if (num_threads < 1) {
																printk(KERN_ERR "Invalid parameter value: num_threads < 1\n");
																counter = 0;
																arr = 0;
																return(-1);
								}
								if (upper_bound < 2) {
																printk(KERN_ERR "Invalid parameter value: upper_bound < 2\n");
																counter = 0;
																arr = 0;
																return(-1);
								}
								counter = kmalloc(num_threads*sizeof(int), GFP_KERNEL);
								if (counter == NULL) {
																printk(KERN_ERR "kmalloc failure!\n");
																counter = NULL;
																arr = NULL;
																return(-1);
								}
								arr = kmalloc((upper_bound+1)*sizeof(int), GFP_KERNEL);
								if (arr == NULL) {
																printk(KERN_ERR "kmalloc failure!\n");
																kfree(counter);
																counter = NULL;
																arr = NULL;
																return(-1);
								}
								int i;
								for (i = 0; i < num_threads; i++) {counter[i] = 0; }
								for (i = 2; i <= upper_bound; i++) {arr[i] = i; }

								spin_lock_init(&cur_lock);
								spin_lock_init(&array_lock);

								curr = 2;

								atomic_set(&num_threads_count, num_threads);
								atomic_set(&num_threads_count1, num_threads);
								struct task_struct *tasks[num_threads];

								int j=0;
								for (; j<num_threads; j++) {
																tasks[j] = kthread_create(thread_fn, (void *)(counter + j), "thread #%d", j);
																wake_up_process(tasks[j]);

								}

								return 0;
}

/* exit function - logs that the module is being removed */
static void
simple_exit(void)
{
								printk(KERN_ALERT "simple module is being unloaded\n");

								while (atomic_read(&num_threads_count) > 0) {
																printk(KERN_ERR "Processing not complete\n");
								}
								if (counter == NULL && arr == NULL) {
																printk(KERN_INFO "Initialization failed\n");
								}
								else {

																unsigned long i = 2;
																unsigned long prime_count = 0;
																printk(KERN_INFO "Printing out the primes\n");
																for (; i<=upper_bound; i++) {

																								if (arr[i] != 0) {
																																prime_count++;
																																printk(KERN_CONT "%d ", arr[i]);
																																if (prime_count %8 == 0) {
																																								printk(KERN_CONT "\n");
																																}
																								}

																}

																printk(KERN_INFO "\n");
																unsigned long count_sum = 0;
																for (i = 0; i < num_threads; i++) {
																								count_sum += counter[i];
																								printk(KERN_INFO "Thread %d: count:%lu\n", i, counter[i]);
																}
																printk(KERN_INFO "prime_count=%lu, num_nonprime=%lu, unnecessary_cross_count=%lu\n", prime_count,upper_bound-prime_count-1,count_sum-upper_bound+prime_count+1);
																printk(KERN_INFO "upper_bound=%lu, num_threads=%lu\n",upper_bound,num_threads);
																printk(KERN_INFO "Time spent setting up the module: %llu, time spent processing primes:%llu\n",times[1].tv64 - times[0].tv64,times[2].tv64 - times[1].tv64);
																kfree((const void *)arr);
																kfree((const void *)counter);
								}

}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Zuohan Xiahou");
MODULE_DESCRIPTION ("CSE 422 Lab 2 Prime Numbers");
