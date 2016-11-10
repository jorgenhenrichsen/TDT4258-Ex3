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
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/signal.h>
#include <asm/siginfo.h>

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
#define DRIVER_NAME "gamepad"
#define GPIO_EVEN_IRQ 17
#define GPIO_ODD_IRQ 18

static int open_gamepad(struct inode* inode, struct file* filp);
static int release_gamepad(struct inode* inode, struct file* filp);
static ssize_t read_gamepad(struct file* filp, char* __user buff, size_t count, loff_t* offp);
static ssize_t write_gamepad(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static irqreturn_t gpio_interrupt_handler(int irq, void * dev_id, struct pt_regs* regs);
static int fasync_gamepad(int fd, struct file *filp, int on);

dev_t devno;
int count;
struct class *cl;
//static unsigned int button1 = *GPIO_PC_DIN;


static struct fasync_struct *gamepad_fasync = NULL;

static struct file_operations gdriv_fops = {
 .owner = THIS_MODULE,
 .open = open_gamepad,
 .release = release_gamepad,
 .read = read_gamepad,
 .write = write_gamepad,
 .fasync = fasync_gamepad
};

struct cdev gamepad_cdev;

static int __init template_init(void)
{
  printk("Gamepad driver init \n");

  devno = MKDEV(15, 20);
  count = 5;

  // TODO: Use alloc_chrdev instead.
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

  /*Setup interrupts*/
  iowrite32(0x22222222, GPIO_EXTIPSELL); // Enable interrupts.
  request_irq(GPIO_EVEN_IRQ, (irq_handler_t)gpio_interrupt_handler, 0, DRIVER_NAME, &gamepad_cdev);
  request_irq(GPIO_ODD_IRQ, (irq_handler_t)gpio_interrupt_handler, 0, DRIVER_NAME, &gamepad_cdev);

  /* Enable iterrupts */
  iowrite32(0xFF, GPIO_EXTIFALL); // Enable interrupts on fall.
  iowrite32(0xFF, GPIO_EXTIRISE); // Enable interrupts on rise.
  iowrite32(0xFF, GPIO_IEN); // Enable GPIO interrupt generation.

  cdev_init(&gamepad_cdev, &gdriv_fops); // init our cdev stucture.
  gamepad_cdev.owner = THIS_MODULE; // Set the cdev owner.
  cdev_add(&gamepad_cdev, devno, count); // Tell kernel about our cdev.

  // Make the driver appear as a file in /dev.
  cl = class_create(THIS_MODULE, DRIVER_NAME); //
  device_create(cl, NULL, devno, NULL, DRIVER_NAME);

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

static int fasync_gamepad(int fd, struct file *filp, int on)
{
  int retval = fasync_helper(fd, filp, on, &gamepad_fasync);

  if (retval < 0)
    return retval;
  return 0;
}


irqreturn_t gpio_interrupt_handler(int irq, void * dev_id, struct pt_regs* regs){

	iowrite32(ioread32(GPIO_IF), GPIO_IFC); // Clear interrupt flag.
  //printk("Driver received interrupt\n");
	/*int buttonPressed = (uint8_t)~(ioread32(GPIO_PC_DIN));
	switch (buttonPressed) {
		case Button1: printk("Button1 \n"); break;
		case Button2: printk("Button2 \n"); break;
		case Button3: printk("Button3 \n"); break;
		case Button4: printk("Button4 \n"); break;
		default: return IRQ_HANDLED;
	}*/

  if (gamepad_fasync){
    kill_fasync(&gamepad_fasync, SIGIO, POLL_IN);
  }
	return IRQ_HANDLED;
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
  copy_to_user(buff, &data, 1);
  return 1;
}

static ssize_t write_gamepad(struct file *filp, const char __user *buff, size_t count, loff_t *offp){
  printk(KERN_INFO "Don't write to the buttons you fools!\n");
  return 1;
}


module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");
