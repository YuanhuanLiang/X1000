/*
 * Simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2006 SWAPP
 *	Andrea Paterniani <a.paterniani@swapp-eng.it>
 * Copyright (C) 2007 David Brownell (simplification, cleanup)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/spi/goodix_fp.h>

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include "spi-goodix-gf5xxx.h"
/*
 * This supports access to SPI devices using normal userspace I/O calls.
 * Note that while traditional UNIX/POSIX I/O semantics are half duplex,
 * and often mask message boundaries, full SPI support requires full duplex
 * transfers.  There are several kinds of internal message boundaries to
 * handle chipselect management and other protocol options.
 *
 * SPI has a character major number assigned.  We allocate minor numbers
 * dynamically using a bitmask.  You must use hotplug tools, such as udev
 * (or mdev with busybox) to create and destroy the /dev/spidevB.C device
 * nodes, since there is no fixed association of minor numbers with any
 * particular SPI bus or device.
 */
#define SPIDEV_MAJOR    153 /* assigned */
#define N_SPI_MINORS    32 /* ... up to 256 */

static DECLARE_BITMAP(minors, N_SPI_MINORS);
static struct spidev_data g_spidev;
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned bufsiz = 30*1024;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

/*-------------------------------------------------------------------------*/

/*
 * We can't use the standard synchronous wrappers for file I/O; we
 * need to protect against async removal of the underlying spi_device.
 */
static void spidev_complete(void *arg)
{
    complete(arg);
}

static ssize_t
spidev_sync(struct spidev_data *spidev, struct spi_message *message)
{
    DECLARE_COMPLETION_ONSTACK(done);
    int status;

    message->complete = spidev_complete;
    message->context = &done;

    spin_lock_irq(&spidev->spi_lock);
    if (spidev->spi == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_async(spidev->spi, message);
    spin_unlock_irq(&spidev->spi_lock);

    if (status == 0) {
        wait_for_completion(&done);
        status = message->status;
        if (status == 0)
            status = message->actual_length;
    }
    return status;
}

static void gf_spi_complete(void *arg)
{
    complete(arg);
}

/**********************************************************
 *Message format:
 *  write cmd   |  ADDR_H |ADDR_L  |  data stream  |
 *    1B         |   1B    |  1B    |  length       |
 *
 * read buffer length should be 1 + 1 + 1 + data_length
 ***********************************************************/
static int gf_spi_write_bytes(struct spidev_data *gf_dev,
    u16 addr, u32 data_len, u8 *tx_buf)
{
    DECLARE_COMPLETION_ONSTACK(read_done);
    struct spi_message msg;
    struct spi_transfer *xfer;
    int ret = 0;

    xfer = kzalloc(sizeof(*xfer), GFP_KERNEL);
    if( xfer == NULL){
        pr_warn("No memory for command.\n");
        return -ENOMEM;
    }

    /*send gf command to device.*/
    spi_message_init(&msg);
    tx_buf[0] = GF_W;
    tx_buf[1] = (u8)((addr >> 8)&0xFF);
    tx_buf[2] = (u8)(addr & 0xFF);
    xfer[0].tx_buf = tx_buf;
    xfer[0].len = data_len + 3;
    xfer[0].delay_usecs = 10;
    spi_message_add_tail(xfer, &msg);
#if 0
    msg.complete = gf_spi_complete;
    msg.context = &read_done;

    spin_lock_irq(&gf_dev->spi_lock);
    ret = spi_async(gf_dev->spi, &msg);
    spin_unlock_irq(&gf_dev->spi_lock);
    if(ret == 0) {
        wait_for_completion(&read_done);
        if(msg.status == 0)
            ret = msg.actual_length - GF_WDATA_OFFSET;
    }
    pr_info("%s ret = %d, actual_length = %d\n", __func__,ret, msg.actual_length);
#else
    spi_sync(gf_dev->spi, &msg);
#endif
    kfree(xfer);
    if(xfer != NULL)
        xfer = NULL;

    return ret;
}


/*************************************************************
 *First message:
 *  write cmd   |  ADDR_H |ADDR_L  |
 *    1B         |   1B    |  1B    |
 *Second message:
 *  read cmd   |  data stream  |
 *    1B        |   length    |
 *
 * read buffer length should be 1 + 1 + 1 + 1 + data_length
 **************************************************************/
static int gf_spi_read_bytes(struct spidev_data *gf_dev,
    u16 addr, u32 data_len, u8 *rx_buf)
{
    DECLARE_COMPLETION_ONSTACK(write_done);
    struct spi_message msg;
    struct spi_transfer *xfer;
    int ret = 0;

    xfer = kzalloc(sizeof(*xfer)*2, GFP_KERNEL);
    if( xfer == NULL){
        pr_warn("No memory for command.\n");
        return -ENOMEM;
    }

    /*send gf command to device.*/
    spi_message_init(&msg);
    rx_buf[0] = GF_W;
    rx_buf[1] = (u8)((addr >> 8)&0xFF);
    rx_buf[2] = (u8)(addr & 0xFF);
    //pr_info("%s === rx_buf[1]:0x%x, rx_buf[2]:0x%x\n",__func__,rx_buf[1],rx_buf[2]);
    xfer[0].tx_buf = rx_buf;
    xfer[0].len = 3;
    xfer[0].delay_usecs = 10;
    spi_message_add_tail(&xfer[0], &msg);

    /*if wanted to read data from gf.
     *Should write Read command to device
     *before read any data from device.
     */
    //memset(rx_buf, 0xff, data_len);
    spi_sync(gf_dev->spi, &msg);
    spi_message_init(&msg);
    memset(rx_buf + GF_RDATA_OFFSET, 0xFF, data_len);
    rx_buf[GF_RDATA_OFFSET - 1] = GF_R;
    xfer[1].tx_buf = &rx_buf[GF_RDATA_OFFSET - 1];

    xfer[1].rx_buf = &rx_buf[GF_RDATA_OFFSET - 1];
    xfer[1].len = data_len + 1;
    xfer[1].delay_usecs = 10;

    spi_message_add_tail(&xfer[1], &msg);
#if 0
    msg.complete = gf_spi_complete;
    msg.context = &write_done;

    spin_lock_irq(&gf_dev->spi_lock);
    ret = spi_async(gf_dev->spi, &msg);
    spin_unlock_irq(&gf_dev->spi_lock);
    if(ret == 0) {
        wait_for_completion(&write_done);
        if(msg.status == 0)
            ret = msg.actual_length - 1;//GF_RDATA_OFFSET;
    }
    pr_info("%s ret = %d, actual_length = %d\n",__func__, ret, msg.actual_length);
#else
    spi_sync(gf_dev->spi, &msg);
#endif
    kfree(xfer);
    if(xfer != NULL)
        xfer = NULL;
    return ret;
}

int gf_spi_write_word(struct spidev_data *gf_dev, unsigned short addr, unsigned short write_value)
{
    gf_dev->buffer[GF_WDATA_OFFSET + 0] =  0;
    gf_dev->buffer[GF_WDATA_OFFSET + 1] =  1;
    gf_dev->buffer[GF_WDATA_OFFSET + 2] =  (unsigned char) (write_value >> 8);
    gf_dev->buffer[GF_WDATA_OFFSET + 3] =  (unsigned char) (write_value & 0xff);

    return gf_spi_write_bytes(gf_dev, addr, 4, gf_dev->buffer);
}

int gf_spi_read_word(struct spidev_data *gf_dev, unsigned short addr, unsigned short *read_value)
{
    int ret = 0;

    ret = gf_spi_read_bytes(gf_dev, addr, 2, gf_dev->buffer);
    *read_value = gf_dev->buffer[GF_RDATA_OFFSET + 0] << 8;
    *read_value += gf_dev->buffer[GF_RDATA_OFFSET + 1];

    return ret;
}


static int gf_spi_transfer(struct spidev_data *gf_dev, unsigned long arg)
{
    struct gf_spi_transfer ioc = {0};
    /*copy command data from user to kernel.*/
    if(copy_from_user(&ioc, (struct gf_spi_transfer*)arg, sizeof(struct gf_spi_transfer))){
        pr_err("Failed to copy command from user to kernel.line=%d\n", __LINE__);
        return -EFAULT;
    }
    if(ioc.len == 0) {
        pr_err("The request length is 0.\n");
        return -EMSGSIZE;
    }

   // mutex_lock(&gf_dev->buf_lock);
     if(ioc.cmd == GF_R) {
        /*if want to read data from hardware.*/
        //pr_info("Read data from 0x%x, len = 0x%x buf = 0x%llx\n", ioc.addr, ioc.len, ioc.buf);
        gf_spi_read_bytes(gf_dev, ioc.addr, ioc.len, gf_dev->buffer);
        if(copy_to_user((void __user*) ioc.buf, (void *)(gf_dev->buffer + GF_RDATA_OFFSET), ioc.len)) {
            pr_err("Failed to copy data from kernel to user.line=%d\n",__LINE__);
           // mutex_unlock(&gf_dev->buf_lock);
            return -EFAULT;
        }
    } else if (ioc.cmd == GF_W) {
        /*if want to read data from hardware.*/
        //pr_info("Write data from 0x%x, len = 0x%x\n", ioc.addr, ioc.len);

        if(copy_from_user(gf_dev->buffer + GF_WDATA_OFFSET, (void *)ioc.buf, ioc.len)){
            pr_err("Failed to copy data from user to kernel.line=%d\n", __LINE__);
            //mutex_unlock(&gf_dev->buf_lock);
            return -EFAULT;
        }

        gf_spi_write_bytes(gf_dev, ioc.addr, ioc.len, gf_dev->buffer);
    } else {
        pr_warn("Error command for gf.\n");
    }
   // mutex_unlock(&gf_dev->buf_lock);
    return 0;
}


static void gf_hw_reset(unsigned int delay_ms)
{
    pr_info("%s %s \n", __FILE__,__func__);
    gpio_direction_output(g_spidev.pdata->reset_pin, 0);
    mdelay(delay_ms);
    gpio_direction_output(g_spidev.pdata->reset_pin, 1);
}

static void gf_enable_irq(struct spidev_data* spidev)
{
    if (spidev->irq_enabled == 1) {
        pr_info("gf_enable_irq  IRQ has been enabled.\n");
        return;
    } else if (spidev->irq_enabled == 0) {
        enable_irq(spidev->irq);
        spidev->irq_enabled = 1;
        pr_info("gf_enable_irq. \n");
    }
}

static void gf_disable_irq(struct spidev_data* spidev)
{
    if (spidev->irq_enabled == 0) {
        pr_info("gf_disable_irq  IRQ has been disabled.\n");
        return;
    } else if (spidev->irq_enabled == 1) {
        disable_irq(spidev->irq);
        spidev->irq_enabled = 0;
        pr_info("gf_disable_irq. \n");
    }
}
static int gf_set_speed(struct spidev_data *spidev, unsigned long speed)
{
    int ret = 0;

    if(speed > 50000000) {
        pr_err("%s speed exceed the max rate. speed = %lu \n",__func__,speed);
        ret = -1;
        return ret;
    }
    //pr_info("%s origin spi is %lu \n",__func__, spidev->spi->max_speed_hz);

    spidev->spi->max_speed_hz = speed;
    spi_setup(spidev->spi);
    //pr_info("%s spi speed to %lu \n",__func__, speed);
    return ret;
}
/*-------------------------------------------------------------------------*/

/* Read-only message with current device setup */
static ssize_t
spidev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    pr_info("%s %s called. \n",__FILE__,__func__);
    return 0;
}


/* Write-only message with current device setup */
static ssize_t
spidev_write(struct file *filp, const char __user *buf,
     size_t count, loff_t *f_pos)
{
    pr_info("%s %s called. \n",__FILE__,__func__);
    return 0;
}

static int gf_fasync(int fd, struct file *filp, int mode)
{
    //struct spidev_data *spidev = filp->private_data;
    struct spidev_data *spidev = &g_spidev;
    int ret;
    ret = fasync_helper(fd, filp, mode, &spidev->async);

    return ret;
}

static long
spidev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int			err = 0;
	int			retval = 0;
	struct spidev_data	*spidev;
	struct spi_device	*spi;
	u32			tmp;
	unsigned		n_ioc;
	struct spi_ioc_transfer	*ioc;
       unsigned long speed = 0;
	unsigned int delay_ms = 0;

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != 'g'/*SPI_IOC_MAGIC*/)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	/* guard against device removal before, or while,
	 * we issue this ioctl.
	 */
	spidev = filp->private_data;
	//spidev = &g_spidev;
	spin_lock_irq(&spidev->spi_lock);
	spi = spi_dev_get(spidev->spi);
	spin_unlock_irq(&spidev->spi_lock);

	if (spi == NULL)
		return -ESHUTDOWN;

	/* use the buffer lock here for triple duty:
	 *  - prevent I/O (from us) so calling spi_setup() is safe;
	 *  - prevent concurrent SPI_IOC_WR_* from morphing
	 *    data fields while SPI_IOC_RD_* reads them;
	 *  - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&spidev->buf_lock);

	switch (cmd) {
		case GF_IOC_DISABLE_IRQ:
			gf_disable_irq(spidev);
			break;
		case GF_IOC_ENABLE_IRQ:
			gf_enable_irq(spidev);
			break;
		case GF_IOC_SETSPEED:
			__get_user(speed, (u32 __user*)arg);
			//pr_info("%s GF_IOC_SETSPEED speed = %lu \n",__func__, speed);
            /* don't set spi speed , goodix will collapse itself when app lib set it*/
			// gf_set_speed(spidev, speed);
			break;
		case GF_IOC_RESET:
			__get_user(delay_ms, (u32 __user*)arg);
			pr_info("%s GF_IOC_RESET delay_ms = %lu \n",__func__, delay_ms);
			gf_hw_reset(delay_ms);
			break;
		case GF_IOC_SPI_TRANSFER:
            //udelay(100);
			//pr_info("%s GF_IOC_SPI_TRNSFER 111\n",__func__);
			gf_spi_transfer(spidev, arg);
			//pr_info("%s GF_IOC_SPI_TRNSFER 222\n",__func__);
			break;
		default:
			break;
	}
	mutex_unlock(&spidev->buf_lock);
	spi_dev_put(spi);
	return retval;
}

#ifdef CONFIG_COMPAT
static long
spidev_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return spidev_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#else
#define spidev_compat_ioctl NULL
#endif /* CONFIG_COMPAT */

static int spidev_open(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = -ENXIO;

	mutex_lock(&device_list_lock);

	list_for_each_entry(spidev, &device_list, device_entry) {
		if (spidev->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status == 0) {
		if (!spidev->buffer) {
			spidev->buffer = kzalloc(bufsiz, GFP_KERNEL);
			if (!spidev->buffer) {
				dev_dbg(&spidev->spi->dev, "open/ENOMEM\n");
				status = -ENOMEM;
			}
		}
		if (status == 0) {
			spidev->users++;
			filp->private_data = spidev;
			nonseekable_open(inode, filp);
			gf_enable_irq(spidev);
		}
	} else
		pr_debug("spidev: nothing for minor %d\n", iminor(inode));

	mutex_unlock(&device_list_lock);

	return status;
}

static int spidev_release(struct inode *inode, struct file *filp)
{
	struct spidev_data	*spidev;
	int			status = 0;

	mutex_lock(&device_list_lock);
	spidev = filp->private_data;
	filp->private_data = NULL;

	/* last close? */
	spidev->users--;
	if (!spidev->users) {
		int		dofree;

		kfree(spidev->buffer);
		spidev->buffer = NULL;

		/* ... after we unbound from the underlying device? */
		spin_lock_irq(&spidev->spi_lock);
		dofree = (spidev->spi == NULL);
		spin_unlock_irq(&spidev->spi_lock);

		if (dofree)
			kfree(spidev);
	}
	mutex_unlock(&device_list_lock);

	return status;
}

static const struct file_operations spidev_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write =	spidev_write,
	.read =		spidev_read,
	.unlocked_ioctl = spidev_ioctl,
	.compat_ioctl = spidev_compat_ioctl,
	.open =		spidev_open,
	.release =	spidev_release,
	.fasync = gf_fasync,
};

/*-------------------------------------------------------------------------*/

/* The main reason to have this class is to make mdev/udev create the
 * /dev/spidevB.C character device nodes exposing our userspace API.
 * It also simplifies memory management.
 */

static struct class *spidev_class;

/*-------------------------------------------------------------------------*/
static irqreturn_t gf_irq(int irq, void *handle) {
    struct spidev_data *spidev= &g_spidev;
    if (spidev->async) {
	//    pr_info("===========%s called.\n",__func__);
        kill_fasync(&spidev->async, SIGIO, POLL_IN);
    }
    return IRQ_HANDLED;
}

static int spidev_probe(struct spi_device *spi)
{
	struct spidev_data	*spidev = &g_spidev;

	int			status;
	unsigned long		minor;
	int irq_request_ret = 0;

	/* Allocate driver data */
	/*spidev = kzalloc(sizeof(*spidev), GFP_KERNEL);
	if (!spidev)
		return -ENOMEM;
	*/

	spidev->pdata = spi->dev.platform_data;
	spidev->irq_enabled = 1;
	spidev->irq = 0;

	/* Initialize the driver data */
	spidev->spi = spi;
	spin_lock_init(&spidev->spi_lock);
	mutex_init(&spidev->buf_lock);

	INIT_LIST_HEAD(&spidev->device_entry);

	/* If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		spidev->devt = MKDEV(SPIDEV_MAJOR, minor);
		/*dev = device_create(spidev_class, &spi->dev, spidev->devt,
				    spidev, "spidev%d.%d",
				    spi->master->bus_num, spi->chip_select);
		*/
		dev = device_create(spidev_class, &spi->dev, spidev->devt,
				    spidev, "goodix_fp");
		status = PTR_RET(dev);
	} else {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
		list_add(&spidev->device_entry, &device_list);
	}
	mutex_unlock(&device_list_lock);

	if (status == 0)
		spi_set_drvdata(spi, spidev);
	else
		kfree(spidev);

	pr_info("%s %s yfpan =========> SPI SETUP mode:0 max_speed_hz:2000000 \n",__FILE__,__func__);
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = 2000000;
	spi->bits_per_word = 8;
	spi_setup(spi);

//yfpan add for test spi start
#if 1
	unsigned char trans_buf[6] = {0};
	unsigned char resv_buf[6] = {0};
      trans_buf[0] = 0xF0;
      trans_buf[1] = 0x01;
	trans_buf[2] = 0x42;
	trans_buf[3] = 0x00;
	trans_buf[4] = 0x01;
	trans_buf[5] = 0xaa;


	struct spi_transfer trans= {
			.tx_buf		= trans_buf,
			.rx_buf		= resv_buf,
			.len		= 5,
		};
	struct spi_message trans_ms;

	spi_message_init(&trans_ms);
	spi_message_add_tail(&trans, &trans_ms);
	spidev_sync(spidev, &trans_ms);

	pr_info("%s resv_buf 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",__func__,resv_buf[0],resv_buf[1],resv_buf[2],resv_buf[3],resv_buf[4],resv_buf[5]);


	//mdelay(300);
	unsigned char trans2_buf[6] = {0};
	unsigned char resv2_buf[6] = {0};
      trans2_buf[0] = 0xF1;
      /*trans2_buf[1] = 0xF1;
	trans2_buf[2] = 0xF2;
	trans2_buf[3] = 0xF3;
	trans2_buf[4] = 0xF4;
	trans2_buf[5] = 0xF5;
	*/

	struct spi_transfer trans2= {
			.tx_buf		= trans2_buf,
			.rx_buf		= resv2_buf,
			.len		= 5,
		};
	struct spi_message trans_ms2;

	spi_message_init(&trans_ms2);
	spi_message_add_tail(&trans2, &trans_ms2);
	spidev_sync(spidev, &trans_ms2);

	pr_err("%s resv_buf 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",__func__,resv2_buf[0],resv2_buf[1],resv2_buf[2],resv2_buf[3],resv2_buf[4],resv2_buf[5]);

#endif //yfpan add for test spi end

/*
  * for X1000 GPIO_PB(18) DEFAULT is low, this pin is suitable for INT pin
  * for X1000 GPIO_PB(16) DEFAULT is high, it is suitable for reset pin
  */
    if(gpio_request(spidev->pdata->pwr_en_pin, "goodix_fp_power_en"))
        pr_err("%s request pwr pin %d FAILED.\n",__func__, spidev->pdata->pwr_en_pin);

    gpio_direction_output(spidev->pdata->pwr_en_pin, 0);

	if(gpio_request(spidev->pdata->reset_pin, "goodix_fp_rst"))
		pr_err("%s request reset pin %d FAILED.\n",__func__, spidev->pdata->reset_pin);

	if(gpio_request(spidev->pdata->int_pin, "gooidx_fp_int"))
		pr_err("%s request irq pin %d FAILED.\n",__func__, spidev->pdata->int_pin);

	spidev->irq = gpio_to_irq(spidev->pdata->int_pin);
	if(spidev->irq < 0)
		pr_err("%s gpio_to_irq FAILED. \n",__func__);

	irq_request_ret = request_threaded_irq(spidev->irq, NULL, gf_irq,
            IRQF_TRIGGER_RISING | IRQF_ONESHOT, "gf", spidev);

	if(0 == irq_request_ret)	 {
		enable_irq_wake(spidev->irq);
		gf_disable_irq(spidev);
	}


	return status;
}

static int spidev_remove(struct spi_device *spi)
{
	struct spidev_data	*spidev = spi_get_drvdata(spi);

	/* make sure ops on existing fds can abort cleanly */
	spin_lock_irq(&spidev->spi_lock);
	spidev->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&spidev->spi_lock);

	/* prevent new opens */
	mutex_lock(&device_list_lock);
	list_del(&spidev->device_entry);
	device_destroy(spidev_class, spidev->devt);
	clear_bit(MINOR(spidev->devt), minors);
	if (spidev->users == 0)
		kfree(spidev);
	mutex_unlock(&device_list_lock);

	return 0;
}

static const struct of_device_id spidev_dt_ids[] = {
	{ .compatible = "rohm,dh2228fv" },
	{},
};

MODULE_DEVICE_TABLE(of, spidev_dt_ids);

static struct spi_driver spidev_spi_driver = {
	.driver = {
		.name =		"goodix_fp",
		.owner =	THIS_MODULE,
		.of_match_table = of_match_ptr(spidev_dt_ids),
	},
	.probe =	spidev_probe,
	.remove =	spidev_remove,

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};

/*-------------------------------------------------------------------------*/

static int __init spidev_init(void)
{
	int status;

	/* Claim our 256 reserved device numbers.  Then register a class
	 * that will key udev/mdev to add/remove /dev nodes.  Last, register
	 * the driver which manages those device numbers.
	 */
	BUILD_BUG_ON(N_SPI_MINORS > 256);
	status = register_chrdev(SPIDEV_MAJOR, "spi", &spidev_fops);
	if (status < 0)
		return status;

	spidev_class = class_create(THIS_MODULE, "spidev");
	if (IS_ERR(spidev_class)) {
		unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
		return PTR_ERR(spidev_class);
	}

	status = spi_register_driver(&spidev_spi_driver);
	if (status < 0) {
		class_destroy(spidev_class);
		unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
	}
	return status;
}
module_init(spidev_init);

static void __exit spidev_exit(void)
{
	spi_unregister_driver(&spidev_spi_driver);
	class_destroy(spidev_class);
	unregister_chrdev(SPIDEV_MAJOR, spidev_spi_driver.driver.name);
}
module_exit(spidev_exit);

MODULE_AUTHOR("Andrea Paterniani, <a.paterniani@swapp-eng.it>");
MODULE_DESCRIPTION("User mode SPI device interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:spidev");
