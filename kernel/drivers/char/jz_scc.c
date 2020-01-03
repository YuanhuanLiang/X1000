/*
 * kernel/drivers/char/jz_scc.c
 *
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * Core file for Ingenic Smart Card Controller  driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/pm.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

#include <mach/jzscc.h>

/**
 * Ioctl commands
 */
#define SCC_MAGIC              'C'
#define SCC_IOC_RST_IIC        _IOW(SCC_MAGIC, 0, int)
#define SCC_IOC_DOWN_IIC       _IOW(SCC_MAGIC, 1, int)
#define SCC_IOC_SEND_ADPU      _IOW(SCC_MAGIC, 2, int)
#define SCC_IOC_G_ADPU_RESP    _IOR(SCC_MAGIC, 3, int)


enum transmode {
    RX,
    TX,
};

enum err_state {
    OK,
    ERROR = !OK,
};

struct scc_drvdata {
    unsigned char atr_count;
    unsigned char atr_table[40];
    struct adpu_cmd adpu;
    struct adpu_response adpu_resp;

    void __iomem *iomem;
    dev_t devno;
    struct cdev cdev;
    struct mutex lock;
    struct class *class;
    struct device *dev;
    struct clk *clk;
    struct resource *iores;
    struct jz_scc_platform_data *pdata;
};

static inline unsigned long scc_reg_read(struct scc_drvdata *scc, int offset)
{
    return readl(scc->iomem + offset);
}

static inline void scc_reg_write(struct scc_drvdata *scc, int offset, unsigned long val)
{
    writel(val, scc->iomem + offset);
}

static inline void scc_reg_or(struct scc_drvdata *scc, int offset, unsigned long val)
{
    unsigned long tmp = readl(scc->iomem + offset);
    writel(tmp | val, scc->iomem + offset);
}

static inline void scc_reg_and(struct scc_drvdata *scc, int offset, unsigned long val)
{
    unsigned long tmp = readl(scc->iomem + offset);
    writel(tmp & val, scc->iomem + offset);
}

static inline void scc_power_on(struct jz_scc_platform_data *pdata)
{
    if (gpio_is_valid(pdata->power_pin))
        gpio_direction_output(pdata->power_pin, pdata->pwr_en_level);
}

static inline void scc_power_off(struct jz_scc_platform_data *pdata)
{
    if (gpio_is_valid(pdata->power_pin))
        gpio_direction_output(pdata->power_pin, !pdata->pwr_en_level);
}

static inline void scc_clk_enable(struct scc_drvdata *scc)
{
    clk_enable(scc->clk);
}

static inline void scc_clk_disable(struct scc_drvdata *scc)
{
    clk_disable(scc->clk);
}

static void scc_transmode(struct scc_drvdata *scc, enum transmode mode)
{
    if (mode == RX) {
        scc_reg_or(scc, SCCCR, EMPTYFIFO);
        scc_reg_and(scc, SCCCR, ~TRANSMODE);
        scc_reg_write(scc, SCCECR, 0xfff);
        scc_reg_and(scc, SCCSR, ~(ECNTO|TRANSEND));
    } else {
        scc_reg_or(scc, SCCCR, EMPTYFIFO);
        scc_reg_write(scc, SCCECR, 0x0);
        scc_reg_and(scc, SCCSR, ~(ECNTO|TRANSEND));
        scc_reg_or(scc, SCCCR, TRANSMODE);
    }
}

static int scc_reset(struct scc_drvdata *scc)
{
    int i, timeout;
    struct jz_scc_platform_data *pdata = scc->pdata;

    printk("===============================\n");
    printk("scc->iomem=0x%08x\n", (unsigned int)scc->iomem);
    printk("scc->pdata->reset_pin=%d\n", scc->pdata->reset_pin);
    printk("===============================\n");

    arch_local_irq_disable();
    scc_power_on(pdata);
    scc_clk_enable(scc);

    /*set etu counter as wait time value = 40000 clk*/
    scc_reg_write(scc, SCCECR, 0xe8);

    gpio_direction_output(pdata->reset_pin, 0);
    scc_reg_or(scc, SCCCR, EMPTYFIFO);
    scc_reg_and(scc, SCCSR, ~(ECNTO|TRANSEND));
    scc_reg_and(scc, SCCCR, ~TRANSMODE);
    scc_reg_or(scc, SCCCR, ENABLE_SCC);
    mdelay(10);
    gpio_direction_output(pdata->reset_pin, 1);

    for (i = 0, timeout = 10000;
        (!(scc_reg_read(scc, SCCSR) & ECNTO) || scc_reg_read(scc, SCCFDR)) && timeout--;
        udelay(50)) {
        if (scc_reg_read(scc, SCCFDR))
            scc->atr_table[i++] = scc_reg_read(scc, SCCDR);
    }
    if (timeout <= 0) {
        dev_err(scc->dev, "smart card reset error, receive ATR timeout\n");
        return -EIO;
    }

    scc->atr_table[i++] = 0;
    scc->atr_count = i;
    return 0;
}

static void scc_disable(struct scc_drvdata *scc)
{
    scc_power_off(scc->pdata);
    scc_reg_and(scc, SCCCR, ~ENABLE_SCC);
    scc_clk_disable(scc);
}

static inline void scc_sendbyte(struct scc_drvdata *scc, unsigned char data)
{
    scc_reg_and(scc, SCCSR, ~TRANSEND);
    scc_reg_write(scc, SCCDR, data);
    while(!(scc_reg_read(scc, SCCSR) & TRANSEND));
}

static int scc_recvbyte(struct scc_drvdata *scc, unsigned char *data)
{
    int timeout;

    for (timeout = 10000;
        !(scc_reg_read(scc, SCCSR) & ECNTO) && timeout--;
        udelay(10)) {
        if (scc_reg_read(scc, SCCFDR)) {
            *data = scc_reg_read(scc, SCCDR);
            scc_reg_and(scc, SCCSR, ~ECNTO);
            return OK;
        }
    }

    dev_err(scc->dev, "failed to recvbyte\n");
    return ERROR;
}

static int scc_sendmsg(struct scc_drvdata *scc)
{
    unsigned char i, recvdata = 0;
    unsigned char check_sw = 0, check_ack = 0;
    struct adpu_cmd *adpu = &scc->adpu;
    struct adpu_response *adpu_resp = &scc->adpu_resp;

    memset(&scc->adpu_resp, 0, sizeof(struct adpu_response));
    scc_transmode(scc, TX);

    /* send header */
    scc_sendbyte(scc, adpu->header.cla);
    scc_sendbyte(scc, adpu->header.ins);
    scc_sendbyte(scc, adpu->header.p1);
    scc_sendbyte(scc, adpu->header.p2);

    /* send body length to/from SC */
    if (adpu->body.lc) {
        scc_sendbyte(scc, adpu->body.lc);
    } else if (adpu->body.le) {
        scc_sendbyte(scc, adpu->body.le);
    }

    scc_transmode(scc, RX);
    if (scc_recvbyte(scc, &recvdata) == OK) {
        check_sw = recvdata & (unsigned char)0xF0;
        check_ack = recvdata & (unsigned char)0xFE;
        if (check_sw == 0x60 || check_sw == 0x90) {
            /* SW1 received */
            adpu_resp->sw1 = recvdata;
            printk("%s: %s -- %d\n",__FILE__, __FUNCTION__, __LINE__);
            if (scc_recvbyte(scc, &recvdata) == OK) {
                /* SW2 received */
                adpu_resp->sw2 = recvdata;
                printk("%s: %s -- %d\n",__FILE__, __FUNCTION__, __LINE__);
            }
        } else if ((check_ack == (((unsigned char)~(adpu->header.ins)) & (unsigned char)0xFE)) ||
            (check_ack == (adpu->header.ins & (unsigned char)0xFE))) {
            adpu_resp->data[0] = recvdata; /* ACK received */
            printk("%s: %s -- %d\n",__FILE__, __FUNCTION__, __LINE__);
        } else {
            printk("%s: %s -- %d\n",__FILE__, __FUNCTION__, __LINE__);
        }
    } else {
        printk("%s: %s -- %d\n",__FILE__, __FUNCTION__, __LINE__);
    }

    if (adpu_resp->sw1 == 0x00) {
        if (adpu->body.lc) {
            /* send body to SC */
            scc_transmode(scc, TX);
            for (i = 0; i < adpu->body.lc; i++) {
                scc_sendbyte(scc, adpu->body.data[i]);
            }
            scc_transmode(scc, RX);
        } else if (adpu->body.le) {
            /* recv data from SC */
            for (i = 0; i < adpu->body.le; i++) {
                if (scc_recvbyte(scc, &recvdata) == OK) {
                    adpu_resp->data[i] = recvdata;
                }
            }
        }

        /* wait SW1...*/
        i = 0;
        while((i++) < 10) {
            if (scc_recvbyte(scc, &recvdata) == OK) {
                adpu_resp->sw1 = recvdata;
                break;
            }
        }
        /* wait SW2...*/
        i = 0;
        while((i++) < 10) {
            if (scc_recvbyte(scc, &recvdata) == OK) {
                adpu_resp->sw1 = recvdata;
                break;
            }
        }
    }

    return 0;
}

static int scc_dev_open(struct inode *inode, struct file *filp)
{
    struct scc_drvdata *scc =
                container_of(inode->i_cdev, struct scc_drvdata, cdev);
    filp->private_data = scc;
    return 0;
}

static int scc_dev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long scc_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    struct scc_drvdata *scc = filp->private_data;
    struct adpu_cmd __user *adpu = (void __user*)arg;

    switch(cmd) {
    case SCC_IOC_RST_IIC:
        retval = scc_reset(scc);
        if (retval < 0)
            return retval;
        if (copy_to_user((void __user *)arg, (void *)scc->atr_table,
                    scc->atr_count)) {
            return -EIO;
        }
        break;

    case SCC_IOC_DOWN_IIC:
        scc_disable(scc);
        break;

    case SCC_IOC_SEND_ADPU:
        if (copy_from_user((void *)&scc->adpu, adpu,
                    sizeof(scc->adpu))) {
            return -EIO;
        }
        retval = scc_sendmsg(scc);
        break;

    case SCC_IOC_G_ADPU_RESP:
        if (copy_to_user((void __user *)arg, (void *)&scc->adpu_resp,
                    sizeof(scc->adpu_resp))) {
            return -EIO;
        }
        break;

    default:
        dev_err(scc->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return retval;
}

static struct file_operations scc_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = scc_dev_open,
    .release = scc_dev_release,
    .unlocked_ioctl = scc_dev_ioctl,
};

static void scc_gpio_free(struct scc_drvdata *scc)
{
    gpio_free(scc->pdata->reset_pin);
    gpio_free(scc->pdata->power_pin);
}

static int scc_gpio_init(struct scc_drvdata *scc)
{
    struct jz_scc_platform_data *pdata = scc->pdata;
    int retval= -ENODEV;

    if (gpio_is_valid(pdata->reset_pin)) {
        retval = gpio_request(pdata->reset_pin, "sc_rst");
        if (retval < 0) {
            dev_err(scc->dev, "request GPIO %d failed\n", pdata->reset_pin);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->reset_pin, 1);
    } else {
        dev_err(scc->dev, "invalid sc reset pin: %d\n", pdata->reset_pin);
        goto err_gpio_request1;
    }

    if (gpio_is_valid(pdata->power_pin)) {
        retval = gpio_request(pdata->power_pin, "sc_pwr");
        if (retval < 0) {
            dev_err(scc->dev, "request GPIO %d failed\n", pdata->power_pin);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->power_pin, !pdata->pwr_en_level);
    }

    return 0;

err_gpio_request2:
    gpio_free(pdata->reset_pin);
err_gpio_request1:
    return retval;
}

static int scc_probe(struct platform_device *pdev)
{
    struct scc_drvdata *scc;
    struct resource *iores;
    int retval;

    if (!pdev->dev.platform_data) {
        dev_err(&pdev->dev, "dev.platform_data cannot be NULL\n");
        retval = -ENODEV;
        goto err_probe;
    }

    scc = kzalloc(sizeof(struct scc_drvdata), GFP_KERNEL);
    if (!scc) {
        dev_err(&pdev->dev, "allocate drvdata memory failed\n");
        retval = -ENOMEM;
        goto err_probe;
    }

    scc->clk = clk_get(NULL, "scc");
    if (IS_ERR(scc->clk)) {
        retval =PTR_ERR(scc->clk);
        dev_err(&pdev->dev, "cannot get scc clock\n");
        goto err_probe;
    }

    iores = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (IS_ERR(iores)) {
        retval = PTR_ERR(iores);
        dev_err(&pdev->dev, "cannot get IORESOURCE_MEM\n");
        goto err_no_iores;
    } else if (!request_mem_region(iores->start, resource_size(iores),
                pdev->name)) {
        retval = -ENXIO;
        dev_err(&pdev->dev, "cannot request iomem region\n");
        goto err_no_iores;
    }

    scc->iomem = ioremap(iores->start, resource_size(iores));
    if (IS_ERR(scc->iomem)) {
        retval = PTR_ERR(scc->iomem);
        dev_err(&pdev->dev, "cannot remap iomem\n");
        goto err_io_remap;
    }

    scc->iores = iores;
    scc->pdata = pdev->dev.platform_data;

    retval =alloc_chrdev_region(&scc->devno, 0, 1, pdev->name);
    if (retval < 0) {
        dev_err(&pdev->dev, "alloc chrdev region failed\n");
        goto err_alloc_chrdev;
    }

    scc->class = class_create(THIS_MODULE, pdev->name);
    if (IS_ERR(scc->class)) {
        retval = PTR_ERR(scc->class);
        dev_err(&pdev->dev, "cannot create class\n");
        goto err_class_create;
    }

    cdev_init(&scc->cdev, &scc_dev_fops);
    scc->cdev.owner = THIS_MODULE;
    scc->cdev.ops = &scc_dev_fops;
    cdev_add(&scc->cdev, scc->devno, 1);

    scc->dev = device_create(scc->class, NULL, scc->devno, scc, pdev->name);
    if (IS_ERR(scc->dev)) {
        retval = PTR_ERR(scc->dev);
        dev_err(&pdev->dev, "cannot create device\n");
        goto err_device_create;
    }

    retval = scc_gpio_init(scc);
    if (retval < 0)
        goto err_gpio_init;

    mutex_init(&scc->lock);
    platform_set_drvdata(pdev, scc);

    dev_info(&pdev->dev, "driver probe successful\n");
    return 0;

err_gpio_init:
    device_destroy(scc->class, 0);
err_device_create:
    class_destroy(scc->class);
err_class_create:
    unregister_chrdev_region(scc->devno, 1);
    cdev_del(&scc->cdev);
err_alloc_chrdev:
    iounmap(scc->iomem);
err_io_remap:
    release_mem_region(iores->start, resource_size(iores));
err_no_iores:
    kfree(scc);
err_probe:
    return retval;
}

static int scc_remove(struct platform_device *pdev)
{
    struct scc_drvdata *scc = platform_get_drvdata(pdev);

    mutex_destroy(&scc->lock);
    device_destroy(scc->class, 0);
    class_destroy(scc->class);
    unregister_chrdev_region(scc->devno, 1);
    cdev_del(&scc->cdev);
    iounmap(scc->iomem);
    release_mem_region(scc->iores->start, resource_size(scc->iores));
    scc_gpio_free(scc);
    kfree(scc);
    return 0;
}

static int scc_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct scc_drvdata *scc = platform_get_drvdata(pdev);

    scc_power_off(scc->pdata);
    return 0;
}

static int scc_resume(struct platform_device *pdev)
{
    struct scc_drvdata *scc = platform_get_drvdata(pdev);

    scc_power_on(scc->pdata);
    return 0;
}

static struct platform_driver scc_driver = {
    .driver = {
        .name  = "jz-scc",
        .owner = THIS_MODULE,
    },
    .probe   = scc_probe,
    .remove  = scc_remove,
    .suspend = scc_suspend,
    .resume  = scc_resume,
};

static int __init scc_init(void)
{
    return platform_driver_register(&scc_driver);
}

static void __exit scc_exit(void)
{
    platform_driver_unregister(&scc_driver);
}

module_init(scc_init);
module_exit(scc_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("jz-scc driver");
