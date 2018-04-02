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

static unsigned long num_threads = 1;
static unsigned long upper_bound = 10;
module_param(num_threads, ulong, 0);
module_param(upper_bound, ulong, 0);

static unsigned int * counter;
static volatile int * arr;
static volatile int curr;
static atomic_t num_threads_finished = ATOMIC_INIT(0);

struct timespec tp1 = {0};
struct timespec tp2 = {0};
struct timespec tp3 = {0};

void lock( volatile int * ptr ){
        int ret_val = 0;
        while (1) {
                ret_val = __atomic_sub_fetch( ptr, 1, __ATOMIC_ACQ_REL );
                if (ret_val >= 0) { break; }
                __atomic_store_n( ptr, -1, __ATOMIC_RELEASE );
                syscall( SYS_futex, ptr, FUTEX_WAIT, -1, NULL );
        }
}

void unlock ( volatile int * ptr ){
        int ret_val = __atomic_add_fetch( ptr, 1, __ATOMIC_ACQ_REL );
        if (ret_val != 1) {
                __atomic_store_n( ptr, 1, __ATOMIC_RELEASE );
                syscall( SYS_futex, ptr, FUTEX_WAKE, INT_MAX );
        }
}

static int
thread_fn(void *void_ptr)
{
        int *counter_ptr = (int *)void_ptr;

        synchronize_barrier();
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tp2);
        mark_nonprimes(counter_ptr);
        synchronize_barrier();
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tp3);
        // TODO: RACE!!!!!!!
}

static void
synchronize_barrier()
{
        atomic_add(1, &num_threads_finished);

        while (atomic_read(&num_threads_finished) > 0) {
                ;
        }

}

static void
mark_nonprimes(int *ptr)
{
        while (1) {
                lock(&curr);
                int prime = curr;
                // increment curr
                while (curr <= upper_bound && arr[curr] == 0) {
                        curr++;
                }
                unlock(&curr);
                if (prime > upper_bound) return;

                int set_zero = prime;
                while (set_zero + prime <= upper_bound) {
                        set_zero += prime;
                        lock(&(arr[set_zero]));
                        arr[set_zero] = 0;
                        unlock(&(unlock arr[set_zero]));
                        // TODO: increment counter
                        (*ptr)++;
                }

        }
}


/* init function - logs that initialization happened, returns success */
static int
simple_init(void)
{
        printk(KERN_ALERT "simple module initialized\n");

        clock_gettime(CLOCK_MONOTONIC_COARSE, &tp1);

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
        if (!counter) {
                printk(KERN_ERR "kmalloc failure!\n");
                counter = 0;
                arr = 0;
                return(-1);
        }
        arr = kmalloc((upper_bound+1)*sizeof(int), GFP_KERNEL);
        if (!arr) {
                printk(KERN_ERR "kmalloc failure!\n");
                kfree(counter);
                counter = 0;
                arr = 0;
                return(-1);
        }
        int i;
        for (i = 0; i < num_threads; i++) {counter[i] = 0; }
        for (i = 2; i <= upper_bound; i++) {arr[i] = i; }
        atomic_set(&curr, 2);

        atomic_set(&num_threads_finished, num_threads);

        struct task_struct *tasks[num_threads];

        int j=0;
        for (; j<num_threads; j++) {
                tasks[j] = kthread_create(thread_fn, (void *)(&(counter[j]), "0"));
                wake_up_process(tasks[j]);

        }

        return 0;
}

/* exit function - logs that the module is being removed */
static void
simple_exit(void)
{
        printk(KERN_ALERT "simple module is being unloaded\n");

        if (num_threads_finished > 0) {
                printk(KERN_ERR "Processing not complete\n");
                return(-1);
        }
        if (counter == 0 && arr == 0) {
                return(-1);
        }


        int i = 0;
        int prime_count = 0;

        for (; i<=upper_bound; i++) {

                if (arr[i] != 0) {
                        prime_count++;
                        printk("%d", arr[i]);
                        if (prime_count %8 == 0) {
                                printk("\n");
                        }
                }

        }
        int count_sum = 0;
        for (i = 0; i < num_threads; i++) count_sum += counter[i];
        printk("\nprime_count=%i, num_nonprime=%i, unnecessary_cross_count=%i\n", pr
               ime_count, (upper_bound-prime_count), (count_sum-upper_bound+prime_count));
        printk("upper_bound=%i, num_threads=%i\n", upper_bound, num_threads);
        double d1, d2;
        d1 = tp2.tv_sec - tp1.tv_sec + 0.000000001*(tp2.tv_nsec - tp1.tv_nsec);
        d2 = tp3.tv_sec - tp2.tv_sec + 0.000000001*(tp3.tv_nsec - tp2.tv_nsec);
        printk("Time spent setting up the module: %f, time spent processing primes:%f\n", d1, d2);

        kfree(arr);
        kfree(counter);

}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Zuohan Xiahou");
MODULE_DESCRIPTION ("CSE 422 Lab 2 Prime Numbers");
