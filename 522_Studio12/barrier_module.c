#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/sort.h>

#define SPIN 1
#define DEFAULT_COUNTER 4
#define KTIME_ARRAY_SIZE 8
#define PRIORITY 10

#define CORE_0 0
#define CORE_1 1
#define CORE_2 2
#define CORE_3 3

static atomic_t barrier;
static ktime_t ktimes[KTIME_ARRAY_SIZE];

static struct task_struct *task0;
static struct task_struct *task1;
static struct task_struct *task2;
static struct task_struct *task3;

static struct sched_param param0;
static struct sched_param param1;
static struct sched_param param2;
static struct sched_param param3;

static int counter = 100;
module_param(counter, int, 0644);

static void setup(int barrier_count, atomic_t * barrier_p){
	atomic_set(barrier_p, barrier_count);
}

static void wait(void){
	atomic_sub_return(1,&barrier);
	while(SPIN){
		if(atomic_read(&barrier)==1) return;
	}
}

static int thread_function(void* data){
	int position;
	ktime_t before, after;
	position=*((int*)data);
	before=ktime_get();
	wait();
	after=ktime_get();
	ktimes[position*2]=before;
	ktimes[position*2+1]=after;
	return 0;
}

static int barrier_init(void){
	int arg0, arg1, arg2, arg3;
	arg0 = 0;
	arg1 = 1;
	arg2 = 2;
	arg3 = 3;

	setup(counter, &barrier);

	task0 = kthread_create(&thread_function,(void *)&arg0,"taks0");
	task1 = kthread_create(&thread_function,(void *)&arg1,"taks1");
	task2 = kthread_create(&thread_function,(void *)&arg2,"taks2");
	task3 = kthread_create(&thread_function,(void *)&arg3,"taks3");

	kthread_bind(task0, CORE_0);
	kthread_bind(task1, CORE_1);
	kthread_bind(task2, CORE_2);
	kthread_bind(task3, CORE_3);

	param0.sched_priority = PRIORITY;
	param1.sched_priority = PRIORITY;
	param2.sched_priority = PRIORITY;
	param3.sched_priority = PRIORITY;

	sched_setscheduler(task0, SCHED_FIFO, &param0);
	sched_setscheduler(task1, SCHED_FIFO, &param1);
	sched_setscheduler(task2, SCHED_FIFO, &param2);
	sched_setscheduler(task3, SCHED_FIFO, &param3);

	wake_up_process(task0);
	wake_up_process(task1);
	wake_up_process(task2);
	wake_up_process(task3);
	return 0;
}

static int cmp_ktime(const void* lhs, const void* rhs){
	ktime_t * left = (ktime_t *)(lhs);
	ktime_t * right = (ktime_t *)(rhs);
	if(left->tv64 < right->tv64) return -1;
	if(left->tv64 > right->tv64) return 1;
	return 0;
}

static void barrier_exit(void){
	int i;
	ktime_t last_prebarrier;
	ktime_t * postbarriers[4];

	last_prebarrier=ktimes[0];
	for (i=1;i<4;i++){
		if(ktimes[i*2].tv64>last_prebarrier.tv64){
			last_prebarrier=ktimes[i*2];
		}
	}
	for (i=0;i<4;i++){
		postbarriers[i]=&(ktimes[i*2+1]);
	}
	sort((void*)postbarriers, 4, sizeof(ktime_t*), &cmp_ktime, 
		NULL);
	printk(KERN_INFO "first post-barrier minus last pre-barrier is %lld",
		postbarriers[0]->tv64-last_prebarrier.tv64);
	printk(KERN_INFO "second post-barrier minus last pre-barrier is %lld",
		postbarriers[1]->tv64-last_prebarrier.tv64);
	printk(KERN_INFO "third post-barrier minus last pre-barrier is %lld",
		postbarriers[2]->tv64-last_prebarrier.tv64);
	printk(KERN_INFO "last post-barrier minus last pre-barrier is %lld",
		postbarriers[3]->tv64-last_prebarrier.tv64);
	
}

module_init(barrier_init);
module_exit(barrier_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zimu Wang, Diqiu Zhou, Wei Weng");
MODULE_DESCRIPTION("Atomic variable barrier latency");
MODULE_VERSION("1.0");
