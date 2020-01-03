/*
 * Ingenic CMOS camera sensor driver only for zk
 *
 * Copyright (C) 2012, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <mach/jz_sensor.h>
#include <linux/regulator/consumer.h>

#define DRIVER_NAME     "sensor"
#define SENSORDRV_LIB_VERSION		"1.0.0 "__DATE__	/* DRV LIB VERSION */
/*
 *	ioctl commands
 */
#define IOCTL_READ_REG            0 /* read sensor registers */
#define IOCTL_WRITE_REG           1 /* write sensor registers */
#define IOCTL_READ_EEPROM         2 /* read  sensor eeprom */
#define IOCTL_WRITE_EEPROM        3 /* write sensor eeprom */
#define IOCTL_SET_ADDR		  4 /* set i2c address */
#define IOCTL_SET_CLK		  5 /* set i2c clock */

#define SIZE  5
struct IO_MSG {
	unsigned int write_size;
	unsigned int read_size;
	unsigned char reg_buf[SIZE];
};

struct eeprom_buf{
        u8 length;
        u8 offset;
        u8 rom[256];
};

struct jz_sensor_dev {
	int i2c_bus_id;
	const char *name;
	struct i2c_client *client;
	struct miscdevice misc_dev;
	struct IO_MSG reg_msg;
	struct sensor_platform_data *pdata;
};

static int sensor_i2c_master_recv(struct i2c_client *client, char *buf, int count)
{
	int ret;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1) {
		printk("ERROR: %s %d jz_sensor_read_reg failed: %d\n", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	return 0;
}

static int sensor_i2c_master_send(struct i2c_client *client, char *buf, int count)
{
	int ret;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret != 1) {
		printk("ERROR: %s %d jz_sensor_write_reg failed: %d\n", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	return 0;
}

static int jz_sensor_read_reg(struct jz_sensor_dev *jz_sensor)
{
	if (sensor_i2c_master_send(jz_sensor->client,
		jz_sensor->reg_msg.reg_buf, jz_sensor->reg_msg.write_size) < 0)
		return -1;
	return sensor_i2c_master_recv(jz_sensor->client,
		jz_sensor->reg_msg.reg_buf, jz_sensor->reg_msg.read_size);
}

static int jz_sensor_write_reg(struct jz_sensor_dev *jz_sensor)
{
	return sensor_i2c_master_send(jz_sensor->client,
		jz_sensor->reg_msg.reg_buf, jz_sensor->reg_msg.write_size);
}

static int jz_sensor_read_eeprom (struct jz_sensor_dev *jz_sensor, u8 *buf, u8 offset, u8 size)
{
	jz_sensor->reg_msg.write_size = size;
	jz_sensor->reg_msg.read_size = 1;
	jz_sensor->reg_msg.reg_buf[0] = offset;

	if (sensor_i2c_master_send(jz_sensor->client,
		jz_sensor->reg_msg.reg_buf, jz_sensor->reg_msg.write_size) < 0)
		return -1;
	return sensor_i2c_master_recv(jz_sensor->client, buf, jz_sensor->reg_msg.read_size);
}

static int jz_sensor_write_eeprom (struct jz_sensor_dev *jz_sensor, u8 *buf, u8 offset, u8 size)
{
	jz_sensor->reg_msg.write_size = size + 1;
	jz_sensor->reg_msg.reg_buf[0] = offset;
	memcpy(&(jz_sensor->reg_msg.reg_buf[1]), buf, size);
	return sensor_i2c_master_send(jz_sensor->client,
		jz_sensor->reg_msg.reg_buf, jz_sensor->reg_msg.write_size);
}

static int sensor_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int sensor_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t sensor_read(struct file *filp, char *buf, size_t size, loff_t *l)
{
	printk("sensor: read is not implemented\n");
	return -1;
}

static ssize_t sensor_write(struct file *filp, const char *buf, size_t size, loff_t *l)
{
	printk("sensor: write is not implemented\n");
	return -1;
}

static long sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	unsigned int i2c_addr;
	struct eeprom_buf rombuf;
	struct miscdevice *dev = filp->private_data;
	struct jz_sensor_dev *jz_sensor = container_of(dev, struct jz_sensor_dev, misc_dev);

	switch (cmd) {
	case IOCTL_READ_REG:
	{
		if (copy_from_user(&jz_sensor->reg_msg, (void *)arg, sizeof(struct IO_MSG)))
			return -EFAULT;

		ret = jz_sensor_read_reg(jz_sensor);
		if (ret)
			return ret;

		if (copy_to_user((void *)arg, &jz_sensor->reg_msg, sizeof(struct IO_MSG)))
			return ret;
		break;
	}

	case IOCTL_WRITE_REG:
	{
		if (copy_from_user(&jz_sensor->reg_msg, (void *)arg, sizeof(struct IO_MSG)))
			return -EFAULT;

		ret = jz_sensor_write_reg(jz_sensor);
		if (ret)
			return ret;
		break;
	}

	case IOCTL_READ_EEPROM:
	{
		if (copy_from_user(&rombuf, (void *)arg, sizeof(rombuf)))
			return -EFAULT;
		ret = jz_sensor_read_eeprom (jz_sensor, &rombuf.rom[0], rombuf.offset, rombuf.length);
		if (ret) {
			printk("ERROR: %s %d jz_sensor_read_eeprom failed: %d\n", __FUNCTION__, __LINE__, ret);
			return ret;
		}
		if (copy_to_user((void *)arg, &rombuf, sizeof(rombuf)))
			return -EFAULT;

		break;
	}

	case IOCTL_WRITE_EEPROM:
	{
		if (copy_from_user(&rombuf, (void *)arg, sizeof(rombuf)))
			return -EFAULT;

		ret = jz_sensor_write_eeprom(jz_sensor, rombuf.rom, rombuf.offset, rombuf.length);
		if (ret)
		{
			printk(" IOCTL_WRITE_EEPROM fail ...\n");
			return ret;
		}
		break;
	}

	case IOCTL_SET_ADDR:
		if (copy_from_user(&i2c_addr, (void *)arg, 4))
			return -EFAULT;
		jz_sensor->client->addr = i2c_addr;
		break;

	case IOCTL_SET_CLK:
		break;

	default:
		printk("Not supported command: 0x%x %s %d\n", cmd, __FUNCTION__, __LINE__);
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations jz_sensor_fops = {
	.owner		= THIS_MODULE,
	.open		= sensor_open,
	.release	= sensor_release,
	.read		= sensor_read,
	.write		= sensor_write,
	.unlocked_ioctl	= sensor_ioctl,
};

/*
 * i2c_driver functions
 */
int sensor_init(struct jz_sensor_dev *jz_sensor)
{
	return jz_sensor->pdata->init(&jz_sensor->client->dev);
}

void sensor_reset(struct jz_sensor_dev *jz_sensor)
{
	jz_sensor->pdata->reset(&jz_sensor->client->dev);
}

void sensor_pwr_on(struct jz_sensor_dev *jz_sensor, int val)
{
	jz_sensor->pdata->power_on(&jz_sensor->client->dev, val);
}

static int jz_sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	int ret = -1;
	struct jz_sensor_dev *jz_sensor;

	printk("SENSOR VERSION =  %s\n", SENSORDRV_LIB_VERSION);
	jz_sensor = kzalloc(sizeof(struct jz_sensor_dev), GFP_KERNEL);
	if (!jz_sensor) {
		ret = -ENOMEM;
		goto exit;
	}

	jz_sensor->client = client;
	jz_sensor->pdata = client->dev.platform_data;
	ret = sensor_init(jz_sensor);
	if (ret < 0)
		return -1;

	sensor_reset(jz_sensor);
	sensor_pwr_on(jz_sensor, 1);

	jz_sensor->i2c_bus_id = client->adapter->nr;
	jz_sensor->name = jz_sensor->pdata->name;

	jz_sensor->misc_dev.minor = MISC_DYNAMIC_MINOR;
	jz_sensor->misc_dev.name = jz_sensor->name;
	jz_sensor->misc_dev.fops = &jz_sensor_fops;
	ret = misc_register(&jz_sensor->misc_dev);
	if (ret < 0)
		goto exit_kfree;

	i2c_set_clientdata(client, jz_sensor);

	printk("JZ %s driver registered, use i2c-%d.\n", jz_sensor->name, jz_sensor->i2c_bus_id);
	return 0;

exit_kfree:
	kfree(jz_sensor);
exit:
	return ret;
}

static int jz_sensor_remove(struct i2c_client *client)
{
	struct jz_sensor_dev *jz_sensor = i2c_get_clientdata(client);
	misc_deregister(&jz_sensor->misc_dev);
	kfree(jz_sensor);
	return 0;
}

static const struct i2c_device_id jz_sensor_id[] = {
	{ "sensor",  0 },
	{ "gc2155",  1 },
	{ "gc0308",  2 },
	{ "ov7725",  3 },
	{ "bf3703",  4 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, jz_sensor_id);

static struct i2c_driver jz_sensor_i2c_driver = {
	.driver = {
		.name = DRIVER_NAME,
	},
	.probe    = jz_sensor_probe,
	.remove   = jz_sensor_remove,
	.id_table = jz_sensor_id,
};

/*
 * Module functions
 */
static int __init jz_sensor_init(void)
{
	return i2c_add_driver(&jz_sensor_i2c_driver);
}

static void __exit jz_sensor_exit(void)
{
	i2c_del_driver(&jz_sensor_i2c_driver);
}

module_init(jz_sensor_init);
module_exit(jz_sensor_exit);

MODULE_DESCRIPTION("JZ camera sensor driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DengYequan <yqdeng@ingenic.cn>");

