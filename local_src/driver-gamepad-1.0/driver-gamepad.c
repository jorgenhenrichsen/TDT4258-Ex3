/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int __init template_init(void)
{
	printk("Hello World, here is your module speaking\n");

	dev_t dev;
	int firstminor = 15;
	int count = 10;
	char name[] = "gamepad";

	int chrdev_status = register_chrdev_region(firstminor, count, name);

	//int chrdev_status = alloc_chrdev_region(&dev, firstminor, count, name);

	printk(KERN_INFO "Chardev reg status: %d \n", chrdev_status);
	printk(KERN_INFO "Chardev devno: %d \n", dev);

	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit template_cleanup(void)
{
	 printk("Short life for a small module...\n");
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");
