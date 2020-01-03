/*
 * chd_pwm_driver.c
 *
 *  Created on: 2015-10-15
 *      Author		: Arthur liang
 * 		Description	: This driver is written for CHD_Q3 development board. You can use it WR/RD gpio
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm-generic/errno-base.h>
#include <linux/cdev.h>
//#include <linux/gpio.h>
//#include <linux/timer.h>
//#include <linux/irq.h>
//#include <linux/stat.h>
#include <linux/device.h>
//#include <linux/interrupt.h>
#include <linux/uaccess.h>		//  copy_from_user() declaration
#include <linux/pwm.h>
#include <linux/delay.h>

#include "chd_pwm.h"

#if DEBUG
#define debug(fmt, arg ...)  printk("[chd_pwm]"fmt, arg)
#else
#define debug(fmt, arg ...) 
#endif

static struct class	*chd_pwm_dev_class;
struct pwm_device *pwmdev[PWM_NUMBER] = {0};

static int chd_pwm_open(struct inode *pinode, struct file *pfile)
{	
	try_module_get(THIS_MODULE);
	return 0;
}

static int chd_pwm_release(struct inode *pinode, struct file *pfile)
{
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t chd_pwm_read(struct file *pfile, char __user *pbuf, size_t size, loff_t *ppos)
{
	return 0;
}

static ssize_t chd_pwm_write(struct file *pfile, const char __user *pbuf, size_t size, loff_t *ppos)
{
	return 0;
}

static int __pwm_set(CHD_PWM_INFO *pwminfo)
{	
	char pwm_label[16];
	int period_ns = 0;
	int duty_ns = 0;
	
	if(pwmdev[pwminfo->pwmidx] == NULL){
		sprintf(pwm_label, "chd_pwm%d", pwminfo->pwmidx);
		
		pwmdev[pwminfo->pwmidx] = pwm_request(pwminfo->pwmidx, pwm_label);//申请pwm设备函数，前面是pwm几，后面是给他的简称
		if(IS_ERR(pwmdev[pwminfo->pwmidx])){
			printk("[chd_pwm] err %ld\n", PTR_ERR(pwmdev[pwminfo->pwmidx]));
			//goto err;
		}
		else{
			printk("[chd_pwm] request pwm0 success.\n");
		}

		pwm_set_polarity(pwmdev[pwminfo->pwmidx], PWM_POLARITY_NORMAL);	/* PWM_POLARITY_NORMAL,  占空比为高电平  PWM_POLARITY_INVERSED, //低电平 */
	}

	period_ns = 1000000000 / pwminfo->frequency; // period(ns) = 1s / frequency
	duty_ns = pwminfo->duty * (period_ns/100);
	//printk("frequency: %d, duty: %d, period_ns: %d  duty_ns: %d\n", pwminfo->frequency, pwminfo->duty, period_ns, duty_ns);

	pwm_disable(pwmdev[pwminfo->pwmidx]);
	pwm_config(pwmdev[pwminfo->pwmidx], duty_ns, period_ns); 
    pwm_enable(pwmdev[pwminfo->pwmidx]);
	
	return 0;
}

static long chd_pwm_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{	
	CHD_PWM_INFO *pwminfo = (CHD_PWM_INFO *)arg;
		
	switch(cmd){	
	case PWM_SET:
		//printk("chird pwm set..\n");
		if(pwminfo->pwmidx < 0 && pwminfo->pwmidx > 4){
			printk("[chd_pwm] Not support this pwm index: %d\n", pwminfo->pwmidx);
			return -EINVAL;
		}

		pwminfo->duty = 100 - pwminfo->duty;
		__pwm_set(pwminfo);	
		break;
	case PWM_GET:
		printk("chird pwm get..\n");
		break;
	default:
		printk("### Unknown command.\n");
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations chd_pwm_ops = {
		.owner 			= THIS_MODULE,
		.open			= chd_pwm_open,
		.release		= chd_pwm_release,
		.unlocked_ioctl	= chd_pwm_ioctl,
		.read			= chd_pwm_read,
		.write			= chd_pwm_write
};

//module initialize function
static int chd_pwm_init(void)
{
	int res = -1;
	
	//initialize
	res = register_chrdev(PWM_MAJOR, PWM_DEVNAME, &chd_pwm_ops);
	if(res < 0){
		printk("can't register major number.\n");
		goto err;
	}

	chd_pwm_dev_class = class_create(THIS_MODULE, "chd_pwm");
	if(IS_ERR(chd_pwm_dev_class)){
		res = PTR_ERR(chd_pwm_dev_class);
		unregister_chrdev(PWM_MAJOR, PWM_DEVNAME);
		printk("class_create failed.\n");
		goto err;
	}

	device_create(chd_pwm_dev_class, NULL, MKDEV(PWM_MAJOR, 0), NULL, PWM_DEVNAME);
	
#if 0
	pwmdev = pwm_request(1, "chd_pwm");//申请pwm设备函数，前面是pwm几，后面是给他的简称
	if(IS_ERR(pwmdev)){
		printk("[chd_pwm] err %ld\n", PTR_ERR(pwmdev));
		//goto err;
	}
	else{
		printk("[chd_pwm] request pwm0 success.\n");
	}
	
	pwm_config(pwmdev, 5000000, 10000000); // 0.5ms
    pwm_set_polarity(pwmdev, PWM_POLARITY_NORMAL);		/* PWM_POLARITY_NORMAL,  占空比为高电平  PWM_POLARITY_INVERSED, //低电平 */
    pwm_enable(pwmdev);
#endif
	printk("[chd_pwm]: module initiation success. \n");
	return 0;
err:
	printk("[chd_pwm]: module initiation failed.\n");
	return -1;
}


//module exit fuc
void chd_pwm_exit(void)
{
	int i = 0;
	for(i = 0; i < PWM_NUMBER; i++){
		if(pwmdev[i] != NULL){
			pwm_disable(pwmdev[i]);
		    pwm_free(pwmdev[i]);
		}
	}
	
	//unregister what we registered
	device_destroy(chd_pwm_dev_class, MKDEV(PWM_MAJOR, 0));
	class_destroy(chd_pwm_dev_class);
	unregister_chrdev(PWM_MAJOR, PWM_DEVNAME);
		
	printk("[chd_pwm]: module exit \n");
}

module_init(chd_pwm_init);
module_exit(chd_pwm_exit);

MODULE_VERSION("V1.0");
MODULE_AUTHOR("Arthur liang");
MODULE_LICENSE("Dual BSD/GPL");

