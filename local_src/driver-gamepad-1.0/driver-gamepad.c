/*
 * This is a demo Linux kernel module.
 */
#include "efm32gg.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/uaccess.h>
/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

 #define Button1 1 << 0
 #define Button2 1 << 1
 #define Button3 1 << 2
 #define Button4 1 << 3

#define DRIVER_NAME "Gamepad_Driver"

 dev_t devno;
 int count;

 //static unsigned int button1 = *GPIO_PC_DIN;



 static struct file_operations gdriv_fops = {
	 .owner = THIS_MODULE,
	 .open = open_gamepad,/*
	 .release = release_gamepad,
	 .read = read_gamepad,
	 .wirte = write_gamepad,*/
 };

static int __init template_init(void)
{
	printk("Gamepad driver init \n");

	devno = MKDEV(15, 20);
	count = 5;



	int status = register_chrdev_region(devno, count, DRIVER_NAME);

	if (status < 0){
		printk("ERROR registering chrdev region: STATUSCODE: %d \n", status);
	}
	else {
		printk("Reigstered chrdev region \n");
	}




	if (request_mem_region(GPIO_PC_DIN, 1, DRIVER_NAME) == NULL){
		printk(KERN_ALERT "Error requesting GPIO_PC_DIN region \n");
	}
	if (request_mem_region(GPIO_PC_DOUT, 1, DRIVER_NAME) == NULL){
		printk(KERN_ALERT "Error requesting GPIO_PC_DOUT region \n");
	}
	if (request_mem_region(GPIO_PC_MODEL, 1, DRIVER_NAME) == NULL){
		printk(KERN_ALERT "Error requesting GPIO_PC_MODEL region \n");
	}

	iowrite32(0x33333333, GPIO_PC_MODEL); // Set pin 0-7 as input.
	iowrite32(0xFF, GPIO_PC_DOUT); // Enable internal pull-up.



	/*int buttonPressed = (uint8_t)~(ioread32(GPIO_PC_DIN));
	printk("%d", buttonPressed);

	switch (buttonPressed) {
		case Button1: printk("Button1"); break;
		case Button2: printk("Button2"); break;
		case Button3: printk("Button3"); break;
		case Button4: printk("Button4"); break;
		default: return; // Go back to deepsleep if another button was pressed.
	}*/

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

	 unregister_chrdev_region(devno, count);

}

static int open_gamepad(struct inode* inode, struct file* filp){
	printk(KERN_INFO "Opening gamepad driver.\n");
	return 0;
}

static int release_gamepad(struct inode* inode, struct file* filp){
	printk(KERN_INFO "Releasing gamepad driver.\n");
	return 0;
}

static ssize_t read_gamepad(struct file* filp, char* __user buff, size_t count, loff_t* offp){
	uint32_t data = ioread32(GPIO_PC_DIN);
	printk("%d", data);
	copy_to_user(buff, &data, 1);
	return 1;
}

static ssize_t write_gamepad(struct file* filp, char* __user buff, size_t count, loff_t* offp){
	printk(KERN_INFO "Don't write to the buttons you fools!\n");
	return 1;
}


module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");
