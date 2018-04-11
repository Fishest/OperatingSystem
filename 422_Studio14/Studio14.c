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
#include <linux/fs_struct.h>
#include <linux/mount.h>
#include <asm/uaccess.h>

static struct task_struct * kthread = NULL;

static int
thread_fn(void * data)
{
    printk(KERN_INFO "Kernel thread started.\n");
    printk(KERN_INFO "fs: %lu, files: %lu, nsproxy: %lu", current->fs, current->files, current->nsproxy);

    struct fs_struct * fs = current->fs;
    printk(KERN_INFO "pwd: %lu, root: %lu", fs->pwd.mnt, fs->root.mnt);
    if (fs->pwd.mnt != fs->root.mnt) {
	printk(KERN_INFO "pwd_mnt: %lu, %lu\n", fs->pwd.mnt->mnt_root, fs->pwd.mnt->mnt_sb);
	printk(KERN_INFO "root_mnt: %lu, %lu\n", fs->root.mnt->mnt_root, fs->root.mnt->mnt_sb);
    }
    printk(KERN_INFO "pwd: %lu, root: %lu", fs->pwd.dentry, fs->root.dentry);
    if (fs->pwd.dentry != fs->root.dentry) {
        printk(KERN_INFO "pwd_dentry: %s\n", fs->pwd.dentry->d_iname);
        printk(KERN_INFO "root_dentry: %s\n", fs->root.dentry->d_iname);
    }
    struct dentry* root = fs->root.dentry;
    struct dentry* cur;
    printk(KERN_INFO "Directories in root\n");
    list_for_each_entry(cur, &(root->d_subdirs), d_child) {
        printk(KERN_INFO "%s\n", cur->d_name.name);
    }
    printk(KERN_INFO "Directories within root\n");
    list_for_each_entry(cur, &(root->d_subdirs), d_child) {
		struct dentry*new_cur;
		printk(KERN_INFO "Under: \n");
		list_for_each_entry(new_cur, &(cur->d_subdirs), d_child) {
        		printk(KERN_INFO "%s\n", new_cur->d_name.name);
    		}
		printk(KERN_INFO "Out!\n");
	printk(KERN_INFO "%s\n", cur->d_name.name);
    }
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

