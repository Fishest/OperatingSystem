#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/string.h>

#define priority 10

static unsigned long core = 0;
static unsigned long period = 3;
static unsigned long iterations = 100;
static char* mode = "FIFO";
module_param(mode, charp, 0644);
module_param(core, ulong, 0644);
module_param(period, ulong, 0644);
module_param(iterations, ulong, 0644);
static struct task_struct * kthread = NULL;
static ktime_t last_run;

void task_fn(void)
{
  ktime_t start_time;
  ktime_t end_time;
  start_time = ktime_get();
  unsigned long i = 0;
  for (i; i < iterations; i++) {
    end_time = ktime_get();
  }
  printk(KERN_INFO "Execution time: %lu\n", (unsigned long) (end_time.tv64-start_time.tv64));
}

enum hrtimer_restart timer_expire(struct hrtimer* timer) {
	last_run = ktime_get();
	task_fn();
	return HRTIMER_RESTART;
}

static int
thread_fn(void * data)
{
  last_run = ktime_get();
  struct hrtimer timer;
  hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
  timer.function = &timer_expire;
  hrtimer_start(&timer, ktime_add(ktime_get(), ktime_set(period, 0)), HRTIMER_MODE_ABS);
  while (!kthread_should_stop()) {
    if (period != 0) {
      hrtimer_forward(&timer, last_run, ktime_set(period, 0));
      schedule();
    }
    else {
      task_fn();
    }
  }
  return 0;
}

static int
kernel_memory_init(void)
{
    printk(KERN_INFO "Loaded kernel_memory module\n");

    kthread = kthread_create(thread_fn, NULL, "studio14");

    kthread_bind(kthread, core);
    int policy;
    if (strncmp(mode, "RR", 2) == 0) {
      policy = SCHED_RR;
    }
    else if (strncmp(mode, "FIFO", 4) == 0) {
      policy = SCHED_FIFO;
    }
    else {
      printk(KERN_INFO "Policy unknown");
      return -1;
    }
    struct sched_param param;
    param.sched_priority = priority;
    sched_setscheduler(kthread, policy, &param);

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
