#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#define KTIME_ARRAY_SIZE 8
#define STAMP_ZERO 0
#define STAMP_ONE 1
#define STAMP_TWO 2
#define STAMP_THREE 3
#define STAMP_FOUR 4
#define STAMP_FIVE 5
#define STAMP_SIX 6
#define STAMP_SEVEN 7
#define HIGH_PRIORITY 10
#define LOW_PRIORITY 1
#define ONE_SECOND 1
#define CORE_ZERO 0

static ktime_t times[KTIME_ARRAY_SIZE];
static struct task_struct *task0;
static struct task_struct *task1;
static struct sched_param param0;
static struct sched_param param1;
static struct hrtimer hr_timer0;
static struct hrtimer hr_timer1;
static ktime_t time0;
static ktime_t time1;

enum hrtimer_restart expire0(struct hrtimer *timer){
	times[STAMP_THREE]=ktime_get();
	printk(KERN_DEBUG "times[3] is %lld.\n", (long long) times[3].tv64);
	wake_up_process(task0);
	return HRTIMER_NORESTART;
}

enum hrtimer_restart expire1(struct hrtimer *timer){
	times[STAMP_SIX]=ktime_get();
	printk(KERN_DEBUG "times[6] is %lld.\n", (long long) times[6].tv64);
	wake_up_process(task0);
	return HRTIMER_NORESTART;
}


static int thread_function1(void* data){
	int i;
	for(i=0;i<20000000;i++){
		ktime_get();
	}
	return 0;
}

static int thread_function0(void* data){
	times[STAMP_FOUR]=ktime_get();
	printk(KERN_DEBUG "times[4] is %lld.\n", (long long) times[4].tv64);
	printk(KERN_DEBUG "Inside thread function.\n");
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();
	times[STAMP_FIVE]=ktime_get();
	printk(KERN_DEBUG "times[5] is %lld.\n", (long long) times[5].tv64);
	param1.sched_priority=LOW_PRIORITY;
	task1=kthread_create(thread_function1, NULL, "low");
	kthread_bind(task1, CORE_ZERO);
	sched_setscheduler(task1, SCHED_FIFO, &param1);
	wake_up_process(task1);
	time1=ktime_set(ONE_SECOND,0);
	hrtimer_init(&hr_timer1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer1.function = &expire1;
	hrtimer_start(&hr_timer1, time1, HRTIMER_MODE_REL);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();
	times[STAMP_SEVEN]=ktime_get();
	printk(KERN_DEBUG "times[7] is %lld.\n", (long long) times[7].tv64);
	return 0;
}


static int timing_init(void){
	int i;
	// Step 2: initialize ktime array
	for(i=0;i<KTIME_ARRAY_SIZE;i++){
		times[i].tv64=0;
	}

	// Step 3: 
	times[STAMP_ZERO]=ktime_get();
	printk(KERN_DEBUG "times[0] is %lld.\n", (long long) times[0].tv64);
	task0=kthread_create(thread_function0, NULL, "high");
	times[STAMP_ONE]=ktime_get();
	printk(KERN_DEBUG "times[1] is %lld.\n", (long long) times[1].tv64);
	param0.sched_priority=HIGH_PRIORITY;
	sched_setscheduler(task0,SCHED_FIFO,&param0);
	times[STAMP_TWO]=ktime_get();
	printk(KERN_DEBUG "times[2] is %lld.\n", (long long) times[2].tv64);

	// Step 4:
	hrtimer_init(&hr_timer0, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer0.function = &expire0;
	time0 = ktime_set(ONE_SECOND, 0);
	kthread_bind(task0, CORE_ZERO);
	hrtimer_start(&hr_timer0, time0, HRTIMER_MODE_REL);
	wake_up_process(task0);
	return 0;
}

static void timing_exit(void){
	int i;
	for(i=0;i<KTIME_ARRAY_SIZE;i++){
		printk(KERN_DEBUG "Timestamp %d is %lld.\n",
			i,(long long) times[i].tv64);
	}
	hrtimer_cancel(&hr_timer0);
	hrtimer_cancel(&hr_timer1);
	printk(KERN_DEBUG "Timestamp 1 minus timestamp 0 equals %lld.\n",
		(long long)(times[1].tv64-times[0].tv64));
	printk(KERN_DEBUG "Timestamp 2 minus timestamp 1 equals %lld.\n",
		(long long)(times[2].tv64-times[1].tv64));
	printk(KERN_DEBUG "Timestamp 4 minus timestamp 0 equals %lld.\n",
		(long long)(times[4].tv64-times[0].tv64));
	printk(KERN_DEBUG "Timestamp 5 minus timestamp 3 equals %lld.\n",
		(long long)(times[5].tv64-times[3].tv64));
	printk(KERN_DEBUG "Timestamp 7 minus timestamp 6 equals %lld.\n",
		(long long)(times[7].tv64-times[3].tv64));
}

module_init(timing_init);
module_exit(timing_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zimu Wang, Diqiu Zhou, Wei Weng");
MODULE_DESCRIPTION("Observing timing events");
MODULE_VERSION("1.0");
