/*
 * File:         byd_fps_spi.c
 *
 * Created:	     2015-11-22
 * Description:  BYD Fingerprint IC driver for Android
 *
 * Copyright (C) 2015 BYD Company Limited
 *
 * Licensed under the GPL-2 or later.
 *
 * History:
 *
 * Contact: qi.ximing@byd.com;
 *
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>  // gpio_set_value()
#include <linux/err.h>   // IS_ERR(), PTR_ERR(); for tiny4412
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/spi/byd_fps.h>

#include "byd_algorithm.h"
#include "byd_fps_bf66xx.h" //#include <linux/input/byd_fps_bf66xx.h>
// -------------------------------------------------------------------- //

#include <linux/types.h>

/*** interface defined by byd_fps_bf663x.c, called by byd_fps_spi.c ***/
struct byd_fps_data *byd_fps_probe(struct spi_device *spi);
void byd_fps_remove(struct byd_fps_data *bydfps,struct spi_device *spi);
int byd_fps_platform_init(struct spi_device *spi);
//int byd_fps_platform_exit(struct device *dev);
int byd_fps_platform_exit(struct spi_device *spi);
void byd_fps_enable_irq_wake(struct spi_device *spi);
void byd_fps_disable_irq_wake(struct spi_device *spi);
#if defined(BYD_FPS_INPUT_WAKEUP) && defined(CONFIG_PM_SLEEP)
extern const struct dev_pm_ops byd_fps_pm_ops;
#elif !defined(CONFIG_PM_SLEEP) && !defined(CONFIG_FB) && !defined(CONFIG_HAS_EARLYSUSPEND)
int byd_fps_suspend(struct spi_device *spi, pm_message_t mesg); // Linux suspend/resume
int byd_fps_resume(struct spi_device *spi);
#endif

// -------------------------------------------------------------------- //

#ifdef CONFIG_FPS03
#define MAX_SPI_FREQ_HZ	18*1000*1000
#elif defined(CONFIG_FPS13)||defined(CONFIG_FPS12)||defined(CONFIG_FPS11)
#define MAX_SPI_FREQ_HZ	24*1000*1000
#else
#define MAX_SPI_FREQ_HZ	50*1000*1000//changed byd cgh for Ingenic , orignal is 12*1000*1000
#endif

#define SPI_WR			1
#define SPI_RD			0

// -------------------------------------------------------------------- //

#ifdef PLATFORM_MTK_KERNEL3_10
		#include <cust_eint.h>
		extern void byd_fps_bf66xx_interrupt(void);
#else
		extern irqreturn_t byd_fps_bf66xx_interrupt(int irq, void *dev_id);
#endif
#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10

#include <linux/irqchip/mt-eic.h>

#ifdef PLATFORM_MTK_KERNEL3_10
    #include <mach/mt_spi.h>	//"mt_spi.h" //struct mt_chip_conf
#else
	#include "mt_spi.h"
#endif 

struct mt_chip_conf spi_conf = {
	.setuptime = 24,
	.holdtime = 24,
	.high_time = 24,
	.low_time = 24,

	.cs_idletime = 24,
	.ulthgh_thrsh = 0,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = SPI_MSB,
	.tx_mlsb = SPI_MSB,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = DMA_TRANSFER,
	.pause = 0,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};

#endif
int byd_fps_spi_speed(struct spi_device *spi, int frq_khz)
{
	int ret = 0;
#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10
	unsigned int half_time;
	half_time = 48*1000/(frq_khz);
	DBG_TIME("%s:calc half_time=%d\n", __func__, half_time);
	half_time = half_time+half_time/5;
	DBG_TIME("%s:half_time adjust=%d\n", __func__, half_time);
	DBG_TIME("%s:Final half_time=%d\n", __func__, half_time);
	spi_conf.setuptime = half_time;
	spi_conf.holdtime = half_time;
	spi_conf.high_time = half_time;
	spi_conf.low_time = half_time;

	spi_conf.cs_idletime = half_time;
	ret = spi_setup(spi);
#elif  defined PLATFORM_SPRD || defined  PLATFORM_INGENIC
	u32 save = spi->max_speed_hz;
	spi->max_speed_hz = frq_khz*1000;	
	ret = spi_setup(spi);
	if(ret < 0) 
		spi->max_speed_hz = save;
#endif
	if (ret < 0)
		pr_warn("bf66xx:Failed to set spi.\n");
	else 
		pr_debug( "%d byd = Hz (max)\n", frq_khz*1000);
	
	return ret;
}

// *******************************************************************************
// * Function    :  byd_fps_spi_xfer
// * Description :  spi transfer.
// * In          :  *spi, count, *tx_buf, *rx_buf.
// * Return      :  0--succeed, -1-- fail.
// *******************************************************************************
int byd_fps_spi_xfer(struct spi_device *spi,
			unsigned int count, unsigned char *tx_buf, unsigned char *rx_buf)
{
	int ret;
	struct spi_message m;

	static struct spi_transfer t = {
		.cs_change = 0,
		.delay_usecs = 0,
		.speed_hz = BYD_FPS_SPI_SPEED,
		.tx_dma = 0,
		.rx_dma = 0,
		.bits_per_word = 8,
	};

	t.speed_hz = spi->max_speed_hz;
	t.tx_buf = tx_buf;
	t.rx_buf = rx_buf;
	t.len = count;

	if (count <= 0)
		return -1;
	//set SPI mode
	#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10
	if (count > 32)
		spi_conf.com_mod = DMA_TRANSFER;
	else
		spi_conf.com_mod = FIFO_TRANSFER;
    #endif

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	ret = spi_sync(spi, &m);
	if (ret) {
		pr_err("byd spi_sync failed.\n");
	}

	return ret;
}

// ---------------------- spi interface ------------------------------- //

/**
 * byd_fps_spi_block_write()
 * @spi: spi device
 * @first_reg: first register to be written to
 * @count: total number of bytes to be written to, plus 1
 * @buf: data to be written to
 *       note: the first byte should be reserved
 *
 * Return: status
 */
int byd_fps_spi_block_write(struct spi_device *spi,
				 u8 first_reg, unsigned int count, u8 *buf)
{
	u8 dummy[count];
	buf[0] = first_reg << 1 | SPI_WR;

	return byd_fps_spi_xfer(spi, count, buf, dummy);
}

/**
 * byd_fps_spi_read()
 * @spi: spi device
 * @reg: the register to be read
 *
 * Return: value of the register or error if less 0
 */
int byd_fps_spi_read(struct spi_device *spi, u8 reg)
{
	int ret;
	#ifdef SPI_TRANS_4BYTE//defined in byd_fps_bf66xx.h
	u8 tx_buf[4], rx_buf[4] = {0, 0,0,0};
	tx_buf[0] = 0XBE;
	tx_buf[1] = 0X00;
	tx_buf[2] = reg << 1 | SPI_RD;
	tx_buf[3] = 0x00;
	ret = byd_fps_spi_xfer(spi, 4, tx_buf, rx_buf)? : rx_buf[3];
	#else
	u8 tx_buf[2], rx_buf[2] = {0, 0};
	tx_buf[0] = reg << 1 | SPI_RD;
	tx_buf[1] = 0x00;
	ret = byd_fps_spi_xfer(spi, 2, tx_buf, rx_buf)? : rx_buf[1];
	#endif

	return ret;
}

// *******************************************************************************
// * Function    :  byd_fps_spi_write
// * Description :  send value to register of fps chip.
// * In          :  @spi: spi device;
// *             :  @reg: the register to be written to;
// *             :  @val: the value to be written to the register;
// * Return      :  status
// *******************************************************************************
int byd_fps_spi_write(struct spi_device *spi, u8 reg, u8 val)
{
	#ifdef SPI_TRANS_4BYTE
	u8 tx_buf[4], dummy[4];
	tx_buf[0] = 0XBE;
	tx_buf[1] = 0X00;
	tx_buf[2] = reg << 1 | SPI_WR;
	tx_buf[3] = val;
	return byd_fps_spi_xfer(spi, 4, tx_buf, dummy);

	#else
	u8 tx_buf[2], dummy[2];
	tx_buf[0] = reg << 1 | SPI_WR;
	tx_buf[1] = val;
	return byd_fps_spi_xfer(spi, 2, tx_buf, dummy);
	#endif
}

// -------------------------------------------------------------------- //
// probe()
// -------------------------------------------------------------------- //

// *******************************************************************************
// * Function    :  byd_fps_spi_probe
// * Description :  spi configure in "probe".
// * In          :  @spi: spi device;
// * Return      :  status
// *******************************************************************************
static int byd_fps_spi_probe(struct spi_device *spi)
{
	struct byd_fps_data  *bydfps = NULL;
	int err;
	
	DBG_TIME("byd_fps_spi_probe start\n");
	DBG_TIME("%s: name:%s, bus_num:%d, num_slave:%d, chip_select:%d, irq:%d spi:%x\n",
			__func__, spi->modalias, spi->master->bus_num, spi->master->num_chipselect,
			 spi->chip_select, spi->irq,spi);

#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10
	spi->mode = SPI_MODE_0; //CPOL=CPHA=0
	spi->controller_data = (void *) &spi_conf;	//setup spi master's mode & frequency
    spi->max_speed_hz = MAX_SPI_FREQ_HZ;
	#ifdef PLATFORM_MTK
	spi->dev.of_node=of_find_compatible_node(NULL, NULL, "byd,bf66xx");
	#if 0
		spidev->reg = regulator_get(&spi->dev, "vfp");
		ret = regulator_set_voltage(spidev->reg, 2800000, 2800000);	/*set 2.8v*/
		if (ret) {
			dev_err("regulator_set_voltage(%d) failed!\n", ret);
			return -1;
		}
	#endif
	#endif
#endif
	/* don't exceed max specified SPI CLK frequency */
	if (spi->max_speed_hz > MAX_SPI_FREQ_HZ) {
		dev_err(&spi->dev, "SPI CLK %d Hz?\n", spi->max_speed_hz);
		return -EINVAL;
	}

	/* --- port setup (gpio, spi, irq, reset) ------------------------------- */
	err = byd_fps_platform_init(spi);
	if(err < 0) {
		dev_err(&spi->dev, "byd_fps_platform_init failed.\n");
		return err;
	}
	byd_fps_spi_speed(spi,1000);//init spi speed

	
	spi->bits_per_word = 8;
	err = spi_setup(spi); // must be after byd_fps_init_port()
	if (err) {
		dev_dbg(&spi->dev, "spi master doesn't support 8 bits/word\n");
		return err;
	}

	bydfps = byd_fps_probe(spi);
	if (IS_ERR(bydfps))
		return PTR_ERR(bydfps);
	//spi_set_drvdata(&spi->dev,bydfps);
	spi_set_drvdata(spi, NULL);
// ---------------------- interrupt init ------------------------------ //

#ifdef PLATFORM_MTK_KERNEL3_10
	mt_eint_set_hw_debounce(spi->irq, 0);
	mt_eint_registration(spi->irq, EINT_TRIGGER_TYPE/* EINTF_TRIGGER_RISING CUST_EINTF_TRIGGER_RISING, CUST_EINTF_TRIGGER_FALLING*/, byd_fps_bf66xx_interrupt, 1);
	//mt_eint_mask(spi->irq);
	//mt_eint_unmask(spi->irq);
#else
	//err = request_irq(spi->irq, byd_fps_bf66xx_interrupt, EINT_TRIGGER_TYPE, spi->dev.driver->name, byd_fps);
	err = request_threaded_irq(spi->irq, NULL, byd_fps_bf66xx_interrupt,
			 EINT_TRIGGER_TYPE | IRQF_ONESHOT, spi->dev.driver->name, NULL);//bydfps
	if (err < 0) {
		pr_err("%s: Fatal Error: request_irq() failed: %d\n", __func__, err);
		return err;
	}
#endif
	//enable_irq(spi->irq);
	byd_fps_enable_irq_wake(spi);//此函数将指纹中断设为能唤醒系统的中断
	dev_dbg(&spi->dev, "%s: irq request OK, irq = %d \n", __func__, spi->irq);
	dev_dbg(&spi->dev, "%s: HZ=%d \n", __func__, HZ);

	return 0;
}

// -------------------------------------------------------------------- //
// remove()
// -------------------------------------------------------------------- //

// *******************************************************************************
// * Function    :  byd_fps_spi_remove
// * Description :  remove resource, clear spi  when driver exit.
// * In          :  @spi: spi device;
// * Return      :  status
// *******************************************************************************
static int byd_fps_spi_remove(struct spi_device *spi)
{
	struct byd_fps_data *bydfps = NULL;

	spi_get_drvdata(spi);
	byd_fps_remove(bydfps,spi);

	if(spi->irq >= 0) {
		byd_fps_disable_irq_wake(spi);
		disable_irq(spi->irq);
		free_irq(spi->irq,NULL );//(void*)0bydfps
	}
	byd_fps_platform_exit(spi);
	kfree(bydfps);
	
	FP_DBG("byd_fps_platform_exit out\n");

	return 0;
}

static struct spi_driver byd_fps_driver = {
	.driver = {
		.name = BYD_FPS_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
	},
	.probe = byd_fps_spi_probe,
	.remove = byd_fps_spi_remove, //.remove = __devexit_p(byd_fps_spi_remove),
	.suspend = byd_fps_suspend,
	.resume = byd_fps_resume,
};


// -------------------------------------------------------------------- //
// module_spi_driver(byd_fps_driver);
// -------------------------------------------------------------------- //

// *******************************************************************************
// * Function    :  byd_fps_init
// * Description :  register spi  driver when module loaded.
// * In          :  void.
// * Return      :  0--succeed, -EINVAL--fail to register spi driver.
// *******************************************************************************
static int __init byd_fps_init(void)
{
	printk("***************************************************\n");
	printk("*           Init BYD Fingerprint Driver  \n");
	printk("*               %s \n", BYD_FPS_NAME);
//	DBG_TIME("* %s \n", BYD_FPS_DRIVER_DESCRIPTION);
	printk("***************************************************\n");
	if (spi_register_driver(&byd_fps_driver)) {
		DBG_TIME("*%s: spi_register_driver failured!\n",__func__);
		return -EINVAL;
	}

	return 0;
}


// *******************************************************************************
// * Function    :  byd_fps_exit
// * Description :  unregister spi  driver when module unloaded.
// * In          :  void.
// * Return      :  void.
// *******************************************************************************
static void __exit byd_fps_exit(void)
{
	printk("%s\n",__func__);
	spi_unregister_driver(&byd_fps_driver);
}

module_init(byd_fps_init);
module_exit(byd_fps_exit);

MODULE_AUTHOR("Simon Chee <qi.ximing@byd.com>");
MODULE_DESCRIPTION("BYD fingerprint chip SPI bus driver");
MODULE_LICENSE("GPL");
