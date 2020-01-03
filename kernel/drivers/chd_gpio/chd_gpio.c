/*
 * chd_gpio_driver.c
 *
 *  Created on: 2015-10-15
 *      Author		: Arthur liang
 * 		Description	: This driver is written for CHD_Q3 development board. You can use it WR/RD gpio
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <asm-generic/errno-base.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>		//  copy_from_user() declaration

#include "chd_gpio.h"

#if DEBUG
#define debug(fmt, arg ...)  printk("[chd_gpio]"fmt, arg)
#else
#define debug(fmt, arg ...) 
#endif

#define __LED_ON(gpio) 		gpio_set_value(gpio, 1);
#define __LED_OFF(gpio)		gpio_set_value(gpio, 0);

struct chd_gpio_info{ // 有效的，被注册的 gpio
	int gpio;		// gpio number
	int io_sel;		// 输入输出状态选择	1 output， 0 input
	int value;		// gpio 初始化状态  1 高电平， 0 低电平
};	

struct chd_gpio_led_status{
	int ticks;				// 节拍, 每 HZ/20 ms一个节拍
	unsigned int ons;		// 每次 LED灯亮 持续的节拍数
	unsigned int offs;		// 每次 LED灯灭 持续的节拍数
	unsigned int resting;	// 
	unsigned int times;		// 灯亮灭的总次数
}gpio_led_stat[GPIO_NUMBER];

static struct class	*gpio_dev_class;
struct timer_list gpio_led_timer;
extern unsigned long volatile jiffies;		// Linux 内核中的全局变量，用来记录自系统启动以来产生中断的总数，启动时初始化为 0
CHD_GPIO_LED_INFO gpio_led_data[GPIO_NUMBER];


struct chd_gpio_info gpio_valid[GPIO_NUMBER]={
	{GPIO_PA(4), 1, 0},
	{GPIO_PA(5), 1, 0},
	{GPIO_PB(21), 1, 0},
};


static void __gpio_led_do_timer(unsigned long data)
{
	// 根据限制值进行判断，控制led灯常亮、常灭、闪烁(闪烁的次数，闪烁时灯亮和灭持续的时间)
	int i;
	unsigned int x;
	for(i = 0; i < GPIO_NUMBER; i++){
		gpio_led_stat[i].ticks++;
		if (gpio_led_data[i].gpio == -1) // -1 means unused
			continue;
		if (gpio_led_data[i].on == GPIO_LED_INFINITY ||
				gpio_led_data[i].off == 0) { //always on, hign level
			__LED_ON(gpio_led_data[i].gpio);
			continue;
		}
		if (gpio_led_data[i].off == GPIO_LED_INFINITY ||
				gpio_led_data[i].rests == GPIO_LED_INFINITY ||
				gpio_led_data[i].on == 0 ||
				gpio_led_data[i].blinks == 0 ||
				gpio_led_data[i].times == 0) { //always off, low level

			__LED_OFF(gpio_led_data[i].gpio);
			continue;
		}

		// led turn on or off
		if (gpio_led_data[i].blinks == GPIO_LED_INFINITY || 
				gpio_led_data[i].rests == 0) { //always blinking
			x = gpio_led_stat[i].ticks % (gpio_led_data[i].on + gpio_led_data[i].off);
		}
		else {
			unsigned int a, b, c, d, o, t;
			a = gpio_led_data[i].blinks / 2;
			b = gpio_led_data[i].rests / 2;
			c = gpio_led_data[i].blinks % 2;
			d = gpio_led_data[i].rests % 2;
			o = gpio_led_data[i].on + gpio_led_data[i].off;
			//t = blinking ticks
			t = a * o + gpio_led_data[i].on * c;
			//x = ticks % (blinking ticks + resting ticks)
			x = gpio_led_stat[i].ticks %
				(t + b * o + gpio_led_data[i].on * d);
			//starts from 0 at resting cycles
			if (x >= t)
				x -= t;
			x %= o;
		}
		if (x < gpio_led_data[i].on) {
			__LED_ON(gpio_led_data[i].gpio);
			if (gpio_led_stat[i].ticks && x == 0)
				gpio_led_stat[i].offs++;
		}
		else {
			__LED_OFF(gpio_led_data[i].gpio);
			if (x == gpio_led_data[i].on)
				gpio_led_stat[i].ons++;
		}

		//blinking or resting
		if (gpio_led_data[i].blinks == GPIO_LED_INFINITY ||
				gpio_led_data[i].rests == 0) { //always blinking
			continue;
		}
		else {
			x = gpio_led_stat[i].ons + gpio_led_stat[i].offs;
			if (!gpio_led_stat[i].resting) {
				if (x == gpio_led_data[i].blinks) {
					gpio_led_stat[i].resting = 1;
					gpio_led_stat[i].ons = 0;
					gpio_led_stat[i].offs = 0;
					gpio_led_stat[i].times++;
				}
			}
			else {
				if (x == gpio_led_data[i].rests) {
					gpio_led_stat[i].resting = 0;
					gpio_led_stat[i].ons = 0;
					gpio_led_stat[i].offs = 0;
				}
			}
		}
		if (gpio_led_stat[i].resting){
			__LED_OFF(gpio_led_data[i].gpio);
		}

		//number of times
		if (gpio_led_data[i].times != GPIO_LED_INFINITY)
		{
			if (gpio_led_stat[i].times ==
					gpio_led_data[i].times) {
				__LED_OFF(gpio_led_data[i].gpio);
				gpio_led_data[i].gpio = -1; //stop
			}
		}
	}

	init_timer(&gpio_led_timer);
	gpio_led_timer.expires = jiffies + HZ/20;		// HZ : 一秒内时钟中断的次数等于 HZ, jiffies + HZ/20表示每次定时50ms
	add_timer(&gpio_led_timer);
}

void __gpio_led_timer_init(void)
{
	int i;
	for(i = 0; i < GPIO_NUMBER; i++){
		gpio_led_data[i].gpio = -1;		// -1 means unused
	}

	// 初始化定时器
	init_timer(&gpio_led_timer);
	gpio_led_timer.function = __gpio_led_do_timer;
	gpio_led_timer.expires = jiffies + HZ/20;	// 定时 50ms
	add_timer(&gpio_led_timer);
}

int __gpio_led_set(CHD_GPIO_LED_INFO led)
{
	int i;
	
	if (0 <= led.gpio){
		
		if (led.on > GPIO_LED_INFINITY)
			led.on = GPIO_LED_INFINITY;
		if (led.off > GPIO_LED_INFINITY)
			led.off = GPIO_LED_INFINITY;
		if (led.blinks > GPIO_LED_INFINITY)
			led.blinks = GPIO_LED_INFINITY;
		if (led.rests > GPIO_LED_INFINITY)
			led.rests = GPIO_LED_INFINITY;
		if (led.times > GPIO_LED_INFINITY)
			led.times = GPIO_LED_INFINITY;

		for(i = 0; i < GPIO_NUMBER; i ++){
			if(gpio_led_data[i].gpio == -1){
				gpio_led_data[i].gpio = led.gpio;
				break;
			}
		}
		
		if (led.on == 0 && led.off == 0 && led.blinks == 0 && led.rests == 0) {
			gpio_led_data[i].gpio = -1; //stop it
			return 0;
		}

		
		//register led data
		gpio_led_data[i].gpio = led.gpio;
		gpio_led_data[i].on = (led.on == 0)? 1 : led.on;
		gpio_led_data[i].off = (led.off == 0)? 1 : led.off;
		gpio_led_data[i].blinks = (led.blinks == 0)? 1 : led.blinks;
		gpio_led_data[i].rests = (led.rests == 0)? 1 : led.rests;
		gpio_led_data[i].times = (led.times == 0)? 1 : led.times;

		//clear previous led status
		gpio_led_stat[i].ticks = -1;
		gpio_led_stat[i].ons = 0;
		gpio_led_stat[i].offs = 0;
		gpio_led_stat[i].resting = 0;
		gpio_led_stat[i].times = 0;
	}
	else{
		debug("\tGPIO%d out of range.\n", led.gpio);
		return -1;
	}

	return 0;
}

int __gpio_request()
{
	int res, i = 0;
	char gpioname[16] = {0};
	for(i = 0; i < GPIO_NUMBER; i ++){
		if(gpio_valid[i].gpio > 0){
			sprintf(gpioname, "gpio%d", gpio_valid[i].gpio);

			res = gpio_request(gpio_valid[i].gpio, gpioname);		// gpio注册
			if (res < 0){
				debug("\tgpio_request %s pin err. \n", gpioname);
				continue;
			}

			if(gpio_valid[i].io_sel == 0){
				gpio_direction_input(gpio_valid[i].gpio);
			}
			else{
				gpio_direction_output(gpio_valid[i].gpio, gpio_valid[i].value);
			}
		}
	}
	
	return 0;
}

void __gpio_free(void)
{
	int i = 0;
		
	for(i = 0; i < GPIO_NUMBER; i ++){
		if(gpio_valid[i].gpio > 0){
			gpio_free(gpio_valid[i].gpio);
			gpio_valid[i].gpio = -1;
		}
	}
}



static int chd_gpio_open(struct inode *pinode, struct file *pfile)
{	
	try_module_get(THIS_MODULE);
	return 0;
}

static int chd_gpio_release(struct inode *pinode, struct file *pfile)
{
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t chd_gpio_read(struct file *pfile, char __user *pbuf, size_t size, loff_t *ppos)
{
	return 0;
}

static ssize_t chd_gpio_write(struct file *pfile, const char __user *pbuf, size_t size, loff_t *ppos)
{
	return 0;
}

static long chd_gpio_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{	
	int result = 0;
	int gpionum = 0;
	int i = 0;
	CHD_GPIO_PORT_INFO *ginfo = NULL;
	CHD_GPIO_LED_INFO led;
	
	switch(cmd){	
	case GPIO_SET_DIR_IN:
		gpionum = (int)arg;
		for(i = 0; i < GPIO_NUMBER; i++){	// check the gpio
			if(gpio_valid[i].gpio == gpionum) 	break;
		}
		
		if(i < GPIO_NUMBER){
			gpio_direction_input(gpionum);
		}
		break;
	case GPIO_SET_DIR_OUT:
		gpionum = (int)arg;	

		for(i = 0; i < GPIO_NUMBER; i++){	// check the gpio
			if(gpio_valid[i].gpio == gpionum) 	break;
		}
		
		if(i < GPIO_NUMBER){
			gpio_direction_output(gpionum, 0);
		}
		break;
	case GPIO_READ_PORT:
		ginfo = (CHD_GPIO_PORT_INFO *)arg;
		gpionum = ginfo->port;

		for(i = 0; i < GPIO_NUMBER; i++){	// check the gpio
			if(gpio_valid[i].gpio == gpionum) 	break;
		}
		if(i < GPIO_NUMBER){
			ginfo->value = gpio_get_value(gpionum);
		}
		break;
	case GPIO_WRITE_PORT:
		ginfo = (CHD_GPIO_PORT_INFO *)arg;
		gpionum = ginfo->port;

		for(i = 0; i < GPIO_NUMBER; i++){	// check the gpio
			if(gpio_valid[i].gpio == gpionum) 	break;
		}
		
		if(i < GPIO_NUMBER){
			gpio_set_value(gpionum, ginfo->value);
		}
		break;
	case GPIO_LED_SET:
		copy_from_user(&led, (CHD_GPIO_LED_INFO *)arg, sizeof(led));
		gpionum = led.gpio;

		for(i = 0; i < GPIO_NUMBER; i++){	// check the gpio
			if(gpio_valid[i].gpio == gpionum) 	break;
		}
		
		if(i < GPIO_NUMBER){
			__gpio_led_set(led);
		}
		break;
	default:
		printk("### Unknown command.\n");
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations chd_gpio_ops = {
		.owner 			= THIS_MODULE,
		.open			= chd_gpio_open,
		.release		= chd_gpio_release,
		.unlocked_ioctl	= chd_gpio_ioctl,
		.read			= chd_gpio_read,
		.write			= chd_gpio_write
};

//module initialize function
static int chd_gpio_init(void)
{
	int res;
	if((res = __gpio_request()) < 0)	goto err;
	
	//initialize
	res = register_chrdev(GPIO_MAJOR, GPIO_DEVNAME, &chd_gpio_ops);
	if(res < 0){
		debug("can't register major number.\n");
		goto err;
	}

	gpio_dev_class = class_create(THIS_MODULE, "chd_gpio");
	if(IS_ERR(gpio_dev_class)){
		res = PTR_ERR(gpio_dev_class);
		unregister_chrdev(GPIO_MAJOR, GPIO_DEVNAME);
		debug("class_create failed.\n");
		goto err;
	}

	device_create(gpio_dev_class, NULL, MKDEV(GPIO_MAJOR, 0), NULL, GPIO_DEVNAME);
	
	__gpio_led_timer_init();	// timer init

	// When the system is starting, the sysled while blink.
	CHD_GPIO_LED_INFO led = {gpio_valid[0].gpio, 5, 5, 2, 1, 8};
	__gpio_led_set(led);
	// When the system is starting, the wifiled while dark
//	gpio_set_value(gpio_valid[1].gpio, 1);

	printk("[chd_gpio]: module initiation success. \n");
	return 0;
err:
	printk("[chd_gpio]: module initiation failed.\n");
	return -1;
}


//module exit fuc
void chd_gpio_exit(void)
{
	__gpio_free();
	
	//unregister what we registered
	device_destroy(gpio_dev_class, MKDEV(GPIO_MAJOR, 0));
	class_destroy(gpio_dev_class);
	unregister_chrdev(GPIO_MAJOR, GPIO_DEVNAME);
	
	del_timer(&gpio_led_timer);
	
	printk("[chd_gpio]: module exit \n");
}

module_init(chd_gpio_init);
module_exit(chd_gpio_exit);

MODULE_VERSION("V1.0");
MODULE_AUTHOR("Arthur liang");
MODULE_LICENSE("Dual BSD/GPL");

