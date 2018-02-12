/*
 * Sample kobject implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 *
 * Released under the GPL version 2 only.
 *
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pid.h>
/*
 * This module shows how to create a simple subdirectory in sysfs called
 * /sys/kernel/kobject-example  In that directory, 3 files are created:
 * "foo", "baz", and "bar".  If an integer is written to these files, it can be
 * later read out of it.
 */

static pid_t userPID;

/*
 * The "foo" file where a static variable is read from and written to.
 */
static ssize_t show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "This shouldn't be used");
}

static ssize_t store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	struct pid* real_pid;
	struct task_struct* pidTask;
	
	int ret;
	ret = kstrtoint(buf, 10, &userPID);
	
	if (ret < 0)
		return ret;
	real_pid = find_vpid(userPID);
	if (real_pid == NULL) {
		printk(KERN_WARNING "vpid failed, returning NULL");
		return count;
	}
	pidTask = pid_task(real_pid, PIDTYPE_PID);
	if (pidTask == NULL) {
		printk(KERN_WARNING "pid_task failed");
		return count;
	}
	int c;
	c = 0;
	while (1 && c < 100) {
		printk(KERN_DEBUG "%s: %d", pidTask->comm, pidTask->pid);
		if (pidTask->pid == 1) {
			break;
		}
		c++;
		pidTask = pidTask->real_parent;	
	}
	if (c == 100) {
		printk("Something is wrong");
	}
	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute pid_attribute =
	__ATTR(userPID, 0664, show, store);

/*
 * More complex function where we determine which variable is being accessed by
 * looking at the attribute for the "baz" and "bar" files.
 */

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&pid_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

static int __init example_init(void)
{
	int retval;

	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	example_kobj = kobject_create_and_add("fam_reader", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);

	return retval;
}

static void __exit example_exit(void)
{
	kobject_put(example_kobj);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
