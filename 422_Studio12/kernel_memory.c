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

static uint nr_structs = 2000;
static uint nr_pages;
static uint order;
static uint nr_structs_per_page;
static struct page * pages;

module_param(nr_structs, uint, 0644); 

typedef struct datatype{
    unsigned int array[8];
} datatype_t;

static struct task_struct * kthread = NULL;

static unsigned int
my_get_order(unsigned int value)
{
    unsigned int shifts = 0;

    if (!value)
        return 0;

    if (!(value & (value - 1)))
        value--;

    while (value > 0) {
        value >>= 1;
        shifts++;
    }

    return shifts;
}

static int
thread_fn(void * data)
{
    //printk("Hello from thread %s. nr_structs=%u\n", current->comm, nr_structs);

    while (!kthread_should_stop()) {
        schedule();
    }

    return 0;
}

static int
kernel_memory_init(void)
{
    printk(KERN_INFO "Loaded kernel_memory module\n");

    kthread = kthread_create(thread_fn, NULL, "k_memory");
    if (IS_ERR(kthread)) {
        printk(KERN_ERR "Failed to create kernel thread\n");
        return PTR_ERR(kthread);
    }
    
    printk(KERN_INFO "Kernel page size %lu\n", PAGE_SIZE);
    printk(KERN_INFO "size of datatype_t: %u\n", sizeof(struct datatype));
    printk(KERN_INFO "%lu can fit\n", PAGE_SIZE/sizeof(struct datatype)); 
    nr_structs_per_page = (unsigned int) PAGE_SIZE/sizeof(struct datatype);
    nr_pages = nr_structs / nr_structs_per_page;
    if (nr_structs % nr_structs_per_page != 0) {
	nr_pages++;
    } 
    order = my_get_order(nr_pages);
    printk(KERN_INFO "nr_structs_per_page: %u, nr_pages: %u, order: %u\n", nr_structs_per_page, nr_pages, order);
    pages = alloc_pages(GFP_KERNEL, order);
    if (pages == NULL) {
	printk(KERN_ERR "Allocating pages failed\n");
	return PTR_ERR(kthread);
    }
    int i, j, k;
    for (i = 0; i < nr_pages; i++) {
	struct page* cur_page = pages + i;
	unsigned long frame_num = page_to_pfn(cur_page);
    	unsigned long pm_addr = PFN_PHYS(frame_num);
    	void * memory_start = __va(pm_addr);
        datatype_t * dt;
        for (j = 0; j < nr_structs_per_page; j++) {
		dt = (datatype_t *) (memory_start + j * sizeof(datatype_t));
		for (k = 0; k < 8; k++) {
			dt->array[k] = i * nr_structs_per_page * 8 + j * 8 + k;
			/*
			if (j == 0 && k == 0) {
				printk(KERN_INFO "num:%d, i:%d, j:%d, k:%d\n", dt->array[k], i, j, k);
			}
			*/
		}
	}
      }
    wake_up_process(kthread);

    return 0;
}

static void 
kernel_memory_exit(void)
{
    kthread_stop(kthread);
	int i, j, k;
	int error = 0;
    for (i = 0; i < nr_pages; i++) {
        struct page* cur_page = pages + i;
        unsigned long frame_num = page_to_pfn(cur_page);
        unsigned long pm_addr = PFN_PHYS(frame_num);
        void * memory_start = __va(pm_addr);
        datatype_t * dt;
        for (j = 0; j < nr_structs_per_page; j++) {
                dt = (datatype_t *) (memory_start + j * sizeof(datatype_t));
                for (k = 0; k < 8; k++) {
                        if (dt->array[k] != i * nr_structs_per_page * 8 + j * 8 + k) {
				printk(KERN_INFO "Mismatch detected\n");
				error = 1;
			}
                }
        }
      }
	if (error == 0) {
		printk(KERN_INFO "Exit successful\n");
	}
    printk(KERN_INFO "Unloaded kernel_memory module\n");
}

module_init(kernel_memory_init);
module_exit(kernel_memory_exit);

MODULE_LICENSE ("GPL");
