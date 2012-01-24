#include <linux/kernel.h>
#include <linux/module.h>
//#include <linux/config.h>
#include <linux/init.h>

#include <linux/fs.h> // register_chrdev

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

//pause
#include <linux/param.h>
#include <asm/system.h>
//#include <asm/proc.h>
//#include <linux/kern.h>

// input configure insmod InFileName="hgjhjh" InVal=76
char *InFileName = ""; // s
//MODULE_PARAM(InFileName, "s"); не нашел определения
int InVal = 0;	 // i
//MODULE_PARAM(InVal, "i");
//short - h, byte - b, long - l

// config
const int CMajor = 251; // if 0 - auto
const char* CName = "symDev";
const unsigned int CGpio = S3C64XX_GPH(9);


//---------------------------------------
// #include "Intf.h"

static void ConfGPH9()
{	
	s3c_gpio_cfgpin(CGpio, S3C_GPIO_OUTPUT); // file .../plat/gpio-cfg.h		
}

static void SetGPH9(bool val)
{
	gpio_set_value(CGpio, val); // arm/.../mach/gpio.h	
}


static 
ssize_t Read (struct file *node, char *user, size_t count, loff_t *offp)
{
	printk("Read %i\n", count);	

	static bool state = true;
	SetGPH9(state);
	state = !state;	
	
	return 0;
}

static
ssize_t Write (struct file *node, const char* user, size_t count, loff_t *offp)
{
	printk("Write \n");
	return 0;
}

static
int Open(struct inode* node, struct file* file)
{
	printk("Open \n");
	//SendReset();
	//MOD_INC_USE_COUNT;
	return 0;
}

static
int Release(struct inode* node, struct file* file) // close ?
{
	printk("Release \n");
	//MOD_DEC_USE_COUNT;
	return 0;
}

//---------------------------------------

static struct file_operations* GetIntf()
{
	static struct file_operations intf = {
		llseek: 0,
		read: Read,
		write: Write,
		ioctl: 0,
		open: Open,
		release: Release
	};
	return &intf;
}

static int TryRegChDev(int major) // return error code
{
	struct file_operations *intf = GetIntf();
	int err = register_chrdev(major, CName, intf);
	if (err < 0) 
	{
		printk("Reg chrdev err:%i\n", err); // how to resolve err?
		return err;
	}
	return 0;
}

static int RegChDev() // return major
{
	struct file_operations *intf = GetIntf();
	int major = register_chrdev(0, CName, intf);
	printk("Drv registred as:%i\n", major);
	return major;
}


//---------------------------------------

int init_module() // insmod
{
	printk("Call init_module\n");

	if (CMajor > 0)
	{
		int err;
		err = TryRegChDev(CMajor); if (err < 0) return err;	
	}
	else
		RegChDev();

	ConfGPH9();	

	
	return 0;
}

void cleanup_module() // rmmod
{
	printk("Call cleanup_module");	
}

MODULE_LICENSE("GPL");
