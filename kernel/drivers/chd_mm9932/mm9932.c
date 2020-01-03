/*
 * mm9932_driver.c
 *
 *  Created on: 2015-10-15
 *      Author		: Arthur liang
 * 		Description	: This driver is written for CHD_Q3 development board. You can use it WR/RD gpio
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/efi.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/slab.h> // kzalloc()
#include "mm9932.h"

#if DEBUG
#define debug(fmt, arg ...)  printk("[mm9932]"fmt, arg)
#else
#define debug(fmt, arg ...) 
#endif

#if 0
/* output voltage table */
#define HIGH_3BIT	8
#define LOW_3BIT	8
float voltage_table[LOW_3BIT][HIGH_3BIT]= {
	0.6,   0.8,   1.0,   1.2,  1.6,  2.0,  2.4, 3.2,
	0.625, 0.825, 1.025, 1.25, 1.65, 2.05, 2.5, 3.3,
	0.65,  0.85,  1.05,  1.3,  1.7,  2.1,  2.6, 3.4,
	0.675, 0.875, 1.075, 1.35, 1.75, 2.15, 2.7, 3.5,
	0.7,   0.9,   1.1,   1.4,  1.8,  2.2,  2.8, 3.6,
	0.725, 0.925, 1.125, 1.45, 1.85, 2.25, 2.9, 3.7,
	0.75,  0.95,  1.15,  1.5,  1.9,  2.3,  3.0, 3.8,
	0.775, 0.975, 1.175, 1.55, 1.95, 2.35, 3.1, 3.9,
};
#endif

static int mm9932_i2c_master_recv(struct i2c_client *client, unsigned char *buf, int count)
{
    int retval;
    struct i2c_msg msg;
    msg.addr  = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.flags |= I2C_M_RD;
    msg.len   = count;
    msg.buf   = buf;

    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static int mm9932_i2c_master_send(struct i2c_client *client, unsigned char *buf, int count)
{
    int retval;
    struct i2c_msg msg;

    msg.addr  = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.len   = count;
    msg.buf   = buf;

    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static inline int mm9932_read_reg(struct i2c_client *client, unsigned char reg)
{
	int ret;
	unsigned char retval;

	ret = mm9932_i2c_master_send(client, &reg, 1);
	if (ret < 0)	return ret;
	if (ret != 1)	return -EIO;

	ret = mm9932_i2c_master_recv(client, &retval, 1);
	if (ret < 0)	return ret;
	if (ret != 1)	return -EIO;
	
	return retval;
}


static inline int mm9932_write_reg(struct i2c_client * client, unsigned char reg, unsigned char val)
{
	unsigned char msg[2];
	int ret;

	msg[0] = reg;
	msg[1] = val;

	ret = mm9932_i2c_master_send(client, msg, 2);

	if (ret < 0)	return ret;
	if (ret < 2)	return -EIO;

	return 0;
}


static inline int mm9932_get_voltage(struct i2c_client *client, int index)
{ 
	unsigned char addr;
	switch(index){ 
		case 1: addr = MM9932_REG1_DCDC_OUT1; break; 
		case 2: addr = MM9932_REG2_DCDC_OUT2; break; 
		case 3: addr = MM9932_REG3_DCDC_OUT3; break; 
		case 4: addr = MM9932_REG4_LDO_OUT4;  break; 
		case 5: addr = MM9932_REG5_LDO_OUT5;  break; 
		case 6: addr = MM9932_REG6_LDO_OUT6;  break; 
		case 7: addr = MM9932_REG7_LDO_OUT7;  break; 
		default: return 0x0;
	} 

	return mm9932_read_reg(client, addr);
}

static inline int mm9932_set_voltage(struct i2c_client *client, int index, unsigned char value)
{ 
	unsigned char addr;
	switch(index){ 
		case 1: addr = MM9932_REG1_DCDC_OUT1; break; 
		case 2: addr = MM9932_REG2_DCDC_OUT2; break; 
		case 3: addr = MM9932_REG3_DCDC_OUT3; break; 
		case 4: addr = MM9932_REG4_LDO_OUT4;  break; 
		case 5: addr = MM9932_REG5_LDO_OUT5;  break; 
		case 6: addr = MM9932_REG6_LDO_OUT6;  break; 
		case 7: addr = MM9932_REG7_LDO_OUT7;  break; 
		default: return 0x0;
	} 

	return mm9932_write_reg(client, addr, value);
}


static inline int mm9932_get_voltage_all(struct i2c_client *client, int *value)
{
	value[0] = mm9932_read_reg(client, MM9932_REG1_DCDC_OUT1);
	value[1] = mm9932_read_reg(client, MM9932_REG2_DCDC_OUT2);
	value[2] = mm9932_read_reg(client, MM9932_REG3_DCDC_OUT3);
	value[3] = mm9932_read_reg(client, MM9932_REG4_LDO_OUT4);
	value[4] = mm9932_read_reg(client, MM9932_REG5_LDO_OUT5);
	value[5] = mm9932_read_reg(client, MM9932_REG6_LDO_OUT6);
	value[6] = mm9932_read_reg(client, MM9932_REG7_LDO_OUT7);
	
	return 0;
}

static int mm9932_dev_open(struct inode *inode, struct file *filp)
{
    struct cdev *cdev = inode->i_cdev;
    struct mm9932_drvdata *mm9932 = container_of(cdev, struct mm9932_drvdata, chd);
	filp->private_data = mm9932;
	
    return 0;
}

static int mm9932_dev_release(struct inode *inode, struct file *filp)
{
    struct mm9932_drvdata *mm9932 = (struct mm9932 *)filp->private_data;

	filp->private_data = NULL;
    return 0;
}

static long mm9932_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct mm9932_drvdata *mm9932 = filp->private_data;
	int ret = 0;
	unsigned int info;
	unsigned int voltage[7];
	unsigned char reg, index, value;

    switch(cmd) {
    case MM9932_GET_UNID:
        break;
    case MM9932_GET_VOL:
		copy_from_user(&info, (unsigned int*)arg, sizeof(unsigned int));
		voltage[0] = mm9932_get_voltage(mm9932->client, info);
		ret = copy_to_user((unsigned char*)arg, voltage, sizeof(unsigned int));
        break;
	case MM9932_GET_VOL_ALL:
		mm9932_get_voltage_all(mm9932->client, voltage);
		copy_to_user((unsigned int*)arg, voltage, sizeof(unsigned int)*7);
		break;
	case MM9932_SET_VOL:
		copy_from_user(&info, (unsigned int*)arg, sizeof(unsigned int));
		value = info     &0xFF;
		index = (info>>8)&0xFF;
		mm9932_set_voltage(mm9932->client, index, value);
		break;
	case MM9932_WRITE_REG:
		copy_from_user(&info, (unsigned int*)arg, sizeof(unsigned int));
		value = info     &0xFF;
		reg   = (info>>8)&0xFF;
		mm9932_write_reg(mm9932->client, reg, value);
		break;
	case MM9932_READ_REG:
		copy_from_user(&info, (unsigned int*)arg, sizeof(unsigned int));
		reg   = (info>>8)&0xFF;
		value = mm9932_read_reg(mm9932->client, reg);
		info = (reg << 8) | value;
		copy_to_user((unsigned char*)arg, &info, sizeof(unsigned int));
		break;
    default:
        dev_err(mm9932->dev, "Not supported commands: 0x%x\n", cmd);
        return -EINVAL;
    }

    return ret;
}

static struct file_operations mm9932_fops = {
    .owner   = THIS_MODULE,
	.unlocked_ioctl	= mm9932_dev_ioctl,
    .open    = mm9932_dev_open,
    .release = mm9932_dev_release,

};

static int mm9932_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	struct mm9932_drvdata *mm9932 = NULL;

	mm9932 = kzalloc(sizeof(struct mm9932_drvdata), GFP_KERNEL);
	if (!mm9932) {
		dev_err(&client->dev, "Failed to allocate drvdata memory\n");
		err = -ENOMEM;
		goto exit_probe;
	}

	mm9932->client 	= client;
	mm9932->dev 	= &client->dev;

	err = alloc_chrdev_region(&mm9932->mid, 0, 1, MM9932_DEVNAME);
	if (err < 0) {
		dev_err(&client->dev, "mm9932 alloc_chrdev_region failed!");
		goto exit_alloc_chrdev_failed;
	}
		
	cdev_init(&mm9932->chd, &mm9932_fops);
	mm9932->chd.owner = THIS_MODULE;
	mm9932->major = MAJOR(mm9932->mid);
    cdev_add(&mm9932->chd, mm9932->mid, 1);

	mm9932->cls = class_create(THIS_MODULE, MM9932_DEVNAME);
	if (IS_ERR(mm9932->cls)) {
		dev_err(&client->dev, "mm9932 class create failed!");
		goto exit_class_create_failed;
	}

	mm9932->dev = device_create(mm9932->cls, &client->dev, mm9932->mid, NULL, MM9932_DEVNAME);
	err = IS_ERR(mm9932->dev) ? PTR_ERR(mm9932->dev) : 0;
	if (err) {
		dev_err(&client->dev, "mm9932 device_create failed. err = %d", err);
		goto exit_device_create_failed;
	}

    i2c_set_clientdata(client, mm9932);

	// enable 5V input IRQ 
	mm9932_write_reg(mm9932->client, 0x78, 0x20);
	mm9932_write_reg(mm9932->client, 0x79, 0x20);

    return 0;
exit_device_create_failed:
    class_destroy(mm9932->cls);
exit_class_create_failed:
    cdev_del(&mm9932->chd);
    unregister_chrdev_region(mm9932->mid, 1);
exit_alloc_chrdev_failed:
	kfree(mm9932);
exit_probe:
    return err;
}

static int mm9932_remove(struct i2c_client *client)
{
    struct mm9932_drvdata *mm9932 = i2c_get_clientdata(client);

    device_destroy(mm9932->cls, mm9932->mid);
    class_destroy(mm9932->cls);
    cdev_del(&mm9932->chd);
    unregister_chrdev_region(mm9932->mid, 1);
	kfree(mm9932);

    return 0;
}

/* when sleep SOC, while run this function */
static int mm9932_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct mm9932_drvdata *mm9932 = i2c_get_clientdata(client);
	mm9932_read_reg(mm9932->client, 0x78);
	mm9932_write_reg(mm9932->client, 0x78, 0x20);
	mm9932_write_reg(mm9932->client, 0x79, 0x20);
    return 0;
}

/* when wakeup SOC, while run this function. */
static int mm9932_resume(struct i2c_client *client)
{
	struct mm9932_drvdata *mm9932 = i2c_get_clientdata(client);
	/* when wakeup the SOC, need read 0x78,after read 0x78, mm9932's nIRQ become HIGH level */
	mm9932_read_reg(mm9932->client, 0x78);	 
	return 0;
}

static const struct i2c_device_id mm9932_id_table[] = {
    { MM9932_DEVNAME, 0 },
    { }
};

static struct i2c_driver mm9932_i2c_driver = {
    .driver = {
        .name = MM9932_DEVNAME,
        .owner = THIS_MODULE,
    },
    .probe    = mm9932_probe,
    .remove   = mm9932_remove,
	.suspend  = mm9932_suspend,
	.resume   = mm9932_resume,
    .id_table = mm9932_id_table,
};

static int __init mm9932_init(void)
{
    return i2c_add_driver(&mm9932_i2c_driver);
}

static void __exit mm9932_exit(void)
{
    i2c_del_driver(&mm9932_i2c_driver);
}

module_init(mm9932_init);
module_exit(mm9932_exit);

MODULE_VERSION("V1.0");
MODULE_AUTHOR("Arthur liang");
MODULE_LICENSE("Dual BSD/GPL");

