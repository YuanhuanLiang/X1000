/*
 * File:         byd_fps_platform.c
 *
 * Created:	     2017-11-16
 * Description:  BYD fingerprint driver - Spreadtrum / Ingenic platform dependent part
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
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "byd_algorithm.h"
#include "byd_fps_bf66xx.h" //#include <linux/input/byd_fps_bf66xx.h>
#include <linux/spi/byd_fps.h>

struct byd_platform_data *byd_pdata = NULL;
/******************************************************************************/
#if defined PLATFORM_SPRD || defined PLATFORM_INGENIC
/******************************************************************************/

/*
 * byd_fps_init_port() - interrupt pin (and sometimes SPI pin) configration
 * @dev:	spi device
 *
 * Return      :  status
 */
//static int byd_fps_init_port(struct device *dev)
static int byd_fps_init_port(struct spi_device *spi)
{
	int ret = 0;

	FP_DBG("%s: entered \n", __func__);

#ifdef BYD_FPS_EINT_PORT
	if (!gpio_is_valid(byd_pdata->int_pin))//BYD_FPS_EINT_PORT
		return -1;
		
	ret = gpio_request(byd_pdata->int_pin, "bf66xx_irq_gpio");//BYD_FPS_EINT_PORT
	if (ret) {
		pr_err("%s: irq port, gpio_request() failed:%d \n", __func__, ret);
		return ret;
	}

	ret = gpio_direction_input(byd_pdata->int_pin);//BYD_FPS_EINT_PORT
	if (ret) {
		pr_err( "%s: irq port, gpio_direction_input() failed:%d \n", __func__, ret);
		gpio_free(byd_pdata->int_pin);//BYD_FPS_EINT_PORT
		return ret;
	}
#endif

#ifdef BYD_FPS_RESET_PORT
	if (!gpio_is_valid(byd_pdata->reset_pin))//BYD_FPS_RESET_PORT
		return -1;
		
	ret = gpio_request(byd_pdata->reset_pin, "bf66xx_reset_gpio");//BYD_FPS_RESET_PORT
	if (ret) {
		pr_err( "%s: reset port, gpio_request() failed:%d \n", __func__, ret);
		return ret;
	}

	ret = gpio_direction_output(byd_pdata->reset_pin, 1);//BYD_FPS_RESET_PORT
	if (ret) {
		pr_err( "%s: reset port, gpio_direction_input() failed:%d \n", __func__, ret);
		gpio_free(byd_pdata->reset_pin);//BYD_FPS_RESET_PORT
		return ret;
	}
#endif

#ifdef BYD_FPS_POWER_PORT
	if (!gpio_is_valid(byd_pdata->pwr_en_pin))//BYD_FPS_POWER_PORT
		return -1;
		
	ret = gpio_request(byd_pdata->pwr_en_pin, "bf66xx_power_gpio");//BYD_FPS_POWER_PORT
	if (ret) {
		pr_err( "%s: power port, gpio_request() failed:%d \n", __func__, ret);
		return ret;
	}

	ret = gpio_direction_output(byd_pdata->pwr_en_pin, 0);//BYD_FPS_POWER_PORT
	if (ret) {
		pr_err( "%s: power port, gpio_direction_input() failed:%d \n", __func__, ret);
		gpio_free(byd_pdata->pwr_en_pin);//BYD_FPS_POWER_PORT
		return ret;
	}
#endif

	return 0;
}


/**
 * byd_fps_platform_init() - platform specific configuration
 * @spi:	spi device
 *
 *  This function is usually used for interrupt pin or SPI pin configration
 * during initialization of FPS driver.
 * Return: irq or error status
 */
int byd_fps_platform_init(struct spi_device *spi)
{
	int ret = 0;
	//dev_set_drvdata(&spi->dev,byd_pdata);
	byd_pdata = kzalloc(sizeof(*byd_pdata), GFP_KERNEL);
	if (!byd_pdata) {
		dev_err(&spi->dev, "failed to allocate memory for struct byd_fps_data\n");
		ret = -ENOMEM;
		return -1;
	}
	byd_pdata = spi->dev.platform_data;
	//ret = byd_fps_init_port(&spi->dev);
	ret = byd_fps_init_port(spi);
	if (ret)
		return ret;

#ifdef BYD_FPS_IRQ_NUM
	spi->irq = BYD_FPS_IRQ_NUM;
	FP_DBG("%s: BYD_FPS_IRQ_NUM is defined as %d\n", __func__, BYD_FPS_IRQ_NUM);

#elif defined BYD_FPS_EINT_PORT
		ret = gpio_to_irq(byd_pdata->int_pin);//BYD_FPS_EINT_PORT
		if (ret < 0) {
			pr_err("%s: gpio_to_irq(%d) failed:%d\n", __func__, byd_pdata->int_pin, ret);
			gpio_free(byd_pdata->int_pin);//BYD_FPS_EINT_PORT
			return ret;
		}

		spi->irq = ret;
		FP_DBG("%s: gpio_to_irq(%d):%d!\n", __func__, byd_pdata->int_pin, ret);
#else
	pr_warn("%s: neither BYD_FPS_IRQ_NUM nor BYD_FPS_EINT_PORT specified\n", __func__);
#endif
	if (spi->irq < 0)
		ret = -1;

	return ret;
}


/**
 * byd_fps_platform_exit() - clean up for platform specific configuration
 * @dev:	pointer to device
 *
 * Return: status
 */
//int byd_fps_platform_exit(struct device *dev)
int byd_fps_platform_exit(struct spi_device *spi)
{
	//byd_pdata = dev_get_drvdata(&spi->dev);
#ifdef BYD_FPS_EINT_PORT
	if(gpio_is_valid(byd_pdata->int_pin)){//BYD_FPS_EINT_PORT
		gpio_free(byd_pdata->int_pin);//BYD_FPS_EINT_PORT
	}
#endif
#ifdef BYD_FPS_RESET_PORT
	if(gpio_is_valid(byd_pdata->reset_pin)){//BYD_FPS_RESET_PORT
		gpio_free(byd_pdata->reset_pin);//BYD_FPS_RESET_PORT
	}
#endif
#ifdef BYD_FPS_POWER_PORT
	if(gpio_is_valid(byd_pdata->pwr_en_pin)){//BYD_FPS_POWER_PORT
		gpio_free(byd_pdata->pwr_en_pin);//BYD_FPS_POWER_PORT
	}
#endif


	FP_DBG("gpio free out!!\n ");
	return 0;
}

#ifdef BYD_FPS_RESET_PORT
int byd_fps_hw_chip_reset(void)
{
	#if 0
	gpio_set_value(byd_pdata->reset_pin, 0);
	mdelay(3);
	gpio_set_value(byd_pdata->reset_pin, 1);
	mdelay(15);
	#endif
	return 0;
}
#endif

#endif // def PLATFORM_SPRD ||  PLATFORM_ENGENIC
