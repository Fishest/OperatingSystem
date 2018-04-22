

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/memory.h>
#include <linux/mm.h>

#include <paging.h>

typedef struct {
    unsigned int nr_pages;
    unsigned long * page_indices;
    atomic_t refcnt;
} vma_tracker_t;

static void
paging_vma_open(struct vm_area_struct * vma)
{
    vma_tracker_t * tracker = (vma_tracker_t *) vma->vm_private_data;
    printk(KERN_INFO "paging_vma_open invoked\n");
    atomic_add(1, &(tracker->refcnt));
}

static void
paging_vma_close(struct vm_area_struct * vma)
{
    if (vma->vm_private_data == NULL) {
      printk(KERN_INFO "Tracker is already null\n");
    }
    else {
      vma_tracker_t * tracker = (vma_tracker_t *) vma->vm_private_data;
      printk(KERN_INFO "paging_vma_close invoked\n");
      atomic_sub(1, &(tracker->refcnt));
      if (atomic_read(&(tracker->refcnt)) == 0) {
        printk(KERN_INFO "Freeing memory\n");
        int i;
        for (i = 0; i < tracker->nr_pages; i++) {
          // free page
          if (tracker->page_indices[i] != 0) {
            unsigned long pfn = tracker->page_indices[i];
            struct page * p = pfn_to_page(pfn);
            __free_pages(p, 0);
          }
        }
        kfree(tracker->page_indices);
        kfree(tracker);
        vma->vm_private_data = NULL;
      }
    }
}

static int
paging_vma_fault(struct vm_area_struct * vma,
                 struct vm_fault       * vmf)
{
    vma_tracker_t * tracker = kmalloc(sizeof(vma_tracker_t), GFP_KERNEL);
    unsigned int offset;
    unsigned long pfn;
    int ret;
    unsigned int i;
    printk(KERN_ERR "We took a page fault, but I don't know how to handle it yet\n");
    printk(KERN_ERR "The VMA covers the VA segment [0x%lx, 0x%lx), and the fault was at VA 0x%lx\n",
        vma->vm_start, vma->vm_end, (unsigned long)vmf->virtual_address
    );
    // allocate memory for the vma_tracker_t structure
    tracker->nr_pages = (unsigned int) ((vma->vm_end - vma->vm_start) / PAGE_SIZE);
    tracker->page_indices = kmalloc(sizeof(unsigned long) * tracker->nr_pages, GFP_KERNEL);

    printk(KERN_INFO "We need %u pages for these memory. Setting them to 0\n", tracker->nr_pages);
    for (i = 0; i < tracker->nr_pages; i++) {
      tracker->page_indices[i] = 0;
    }
    atomic_set(&(tracker->refcnt), 1);
    vma->vm_private_data = (void *) tracker;

    // calculate offset
    offset = (unsigned long) (vmf->virtual_address - vma->vm_start) / PAGE_SIZE;
    // one page
    struct page * p = alloc_pages(GFP_KERNEL, 0);
    if (p == NULL) {
      printk(KERN_ERR "Alloc page failed\n");
      return VM_FAULT_OOM;
    }
    pfn = page_to_pfn(p);
    ret = remap_pfn_range(vma, (unsigned long) PAGE_ALIGN((unsigned long)vmf->virtual_address), pfn, PAGE_SIZE, vma->vm_page_prot);
    // store if successful
    if (ret == 0) {
      tracker->page_indices[offset] = pfn;
    }
    else {
      printk(KERN_ERR "Remap pfn range failed.\n");
      return VM_FAULT_SIGBUS;
    }

    return VM_FAULT_NOPAGE;
}

static struct vm_operations_struct
paging_vma_ops =
{
    .open = paging_vma_open,
    .close = paging_vma_close,
    .fault = paging_vma_fault,
};


/* vma is the new virtual address segment for the process */
static int
paging_mmap(struct file           * filp,
            struct vm_area_struct * vma)
{

    /* prevent Linux from mucking with our VMA (expanding it, merging it
     * with other VMAs, etc.
     */
    vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_NORESERVE | VM_DONTDUMP | VM_PFNMAP;

    /* setup the vma->vm_ops, so we can catch page faults on this vma */
    vma->vm_ops = &paging_vma_ops;

    return 0;
}

static struct file_operations
dev_ops =
{
    .mmap = paging_mmap,
};

static struct miscdevice
dev_handle =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = PAGING_MODULE_NAME,
    .fops = &dev_ops,
};

/*** Kernel module initialization and teardown ***/
static int
paging_init(void)
{
    int status;

    /* Create a character device to communicate with user-space via file I/O operations */
    status = misc_register(&dev_handle);
    if (status != 0)
    {
        printk(KERN_ERR "Failed to register misc. device for module\n");
        return status;
    }

    printk(KERN_INFO "Loaded paging module\n");

    return 0;
}

static void
paging_exit(void)
{
    /* Deregister our device file */
    misc_deregister(&dev_handle);

    printk(KERN_INFO "Unloaded paging module\n");
}

module_init(paging_init);
module_exit(paging_exit);

/* Misc module info */
MODULE_LICENSE("GPL");
