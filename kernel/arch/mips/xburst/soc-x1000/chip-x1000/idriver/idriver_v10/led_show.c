#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <board.h>
#define DEVCIE_NAME "led_show"
#define SUCCESS	0
//#define debug printk
#define debug(...) //printk
static int Device_Open;
static unsigned long led_bitmap = 0;
static unsigned long scan_time = 100;
static int device_open(struct inode *inode, struct file *file)
{
	debug("%s\n",__FUNCTION__);
	if(Device_Open)
		return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;
	module_put(THIS_MODULE);
	return 0;
}

long device_ioctl (struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	debug("set led ioctl:num:%d,%ld\n",ioctl_num, ioctl_param);
	switch(ioctl_num){
		case 0:
			led_bitmap = ioctl_param;
			break;
		case 1:
			scan_time = ioctl_param;
			break;
		default:
			break;
	}
	return SUCCESS;
}

#define MATRIX_ROW	3
#define MATRIX_COLUMN	4
static unsigned int led_column[MATRIX_COLUMN]={
	LED_N1,
	LED_N2,
	LED_N3,
	LED_N4,
};
static unsigned int led_row[MATRIX_ROW]={
	LED_P1,
	LED_P2,
	LED_P3,
};
static int led_matrix_init(void)
{
	int ret;
	int i;
	debug("%s\n",__FUNCTION__);
	ret = gpio_request(LED_P3, "lcd p3");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_P2, "lcd p2");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_P1, "lcd p1");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_N4, "lcd n4");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_N3, "lcd n3");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_N2, "lcd n2");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	ret = gpio_request(LED_N1, "lcd n1");
	if (ret) {
		printk(KERN_ERR "can't request led gpio\n");
		return ret;
	}
	for(i=0; i<MATRIX_ROW; i++)
	{
		gpio_direction_output(led_row[i], 0);
	}
	for(i=0; i<MATRIX_COLUMN; i++)
	{
		gpio_direction_output(led_column[i], 1);
	}
	return 0;
}

static int scan_row;
static int scan_delay;
static int led_matrix_scan(void)
{
	int i;
	if(scan_delay)
	{
		gpio_direction_output(led_row[scan_row], 0);
		scan_delay = 0;
		return 0;
	}
	scan_row++;
	if(scan_row >= MATRIX_ROW)
		scan_row = 0;
	for(i=0; i<MATRIX_COLUMN; i++)
	{
		if(led_bitmap &(1<<(scan_row+i*MATRIX_ROW))){
			gpio_direction_output(led_column[i], 0);
		}
		else{
			gpio_direction_output(led_column[i], 1);
		}
	}
	gpio_direction_output(led_row[scan_row], 1);
	scan_delay = 1;
	return 0;
}

static ktime_t ktime;
static struct hrtimer hr_timer;
enum hrtimer_restart led_timer_callback( struct hrtimer* hrtimer)
{
	led_matrix_scan();
	hrtimer_forward(&hr_timer, hrtimer_cb_get_time(&hr_timer), ktime);
	return HRTIMER_RESTART;
}

static struct file_operations fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};


static int led_major;
module_param(led_major, int, 0);
static struct class *led_class = NULL;
struct cdev *led_cdev;
struct device *led_device;
#define SCAN_NSEC	(1000*1000)
#define DEVICE_NAME "led_show"
static int init_led_timer(void)
{
	ktime = ktime_set( 0, SCAN_NSEC );
	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timer.function = led_timer_callback;
	printk("init led timer\n");

	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
	return 0;
}

static void led_cleanup_module(void)
{
	if (led_cdev) {
		cdev_del(led_cdev);
		kfree(led_cdev);
	}
	if (led_device) {
		device_destroy(led_class,MKDEV(led_major, 0));
	}

	if (led_class)
		class_destroy(led_class);

	unregister_chrdev_region(MKDEV(led_major, 0), 1);
	return;
}
int init_module(void)
{
	int ret;
	dev_t dev = MKDEV(led_major, 0);
	ret = alloc_chrdev_region(&dev, 0, 1, "led");
	if(ret < 0){
		printk("alloc chardev err\n");
		return ret;
	}
	led_major = MAJOR(dev);
	led_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(led_class)){
		printk("class create err\n");
		goto err;
	}
	led_cdev = cdev_alloc();
	if(!led_cdev){
		printk("alloc led cdev err\n");
		goto err;
	}
	cdev_init(led_cdev, &fops);
	ret = cdev_add(led_cdev, dev, 1);
	if(ret < 0){
		printk("cdev add err\n");
		return ret;
	}
	led_device = device_create(led_class, NULL, dev, NULL, DEVICE_NAME);
	if(IS_ERR(led_device)){
		printk("device create err\n");
		cdev_del(led_cdev);
		goto err;
	}

	led_matrix_init();
	init_led_timer();
	return SUCCESS;
err:
	led_cleanup_module();
	return ret;
}

void cleanup_module(void)
{
	led_cleanup_module();
	hrtimer_cancel( &hr_timer );
	debug(KERN_INFO "led show cleanup.\n");
}


module_init(init_module);
module_exit(cleanup_module);

MODULE_LICENSE("GPL");
