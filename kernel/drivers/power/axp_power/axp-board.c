/*
 * board-power-619.c
 *
 * Copyright (C) 2012-2014, Ricoh Company,Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
#include <linux/io.h>
#include "axp-board.h"
#include "axp-cfg.h"

#define PMU_IRQ_N       GPIO_PD(2)

struct i2c_board_info __initdata  axp_regulator   = {
	I2C_BOARD_INFO("axp_mfd", AXP_DEVICES_ADDR),
	.irq		= PMU_IRQ_N,
	.platform_data	= &axp_pdata,
};

static int __init pmu_dev_init(void)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	int busnum = AXP_I2CBUS;
	int i;

	struct regulator* regulator;
//	return 0;

	if (gpio_request_one(PMU_IRQ_N,
				GPIOF_DIR_IN, "axp_irq")) {
		pr_err("The GPIO %d is requested by other driver,"
				" not available for axp\n", PMU_IRQ_N);
		axp_regulator.irq = -1;
	} else {
		axp_regulator.irq = gpio_to_irq(PMU_IRQ_N);
	}

	adap = i2c_get_adapter(busnum);
	if (!adap) {
		pr_err("failed to get adapter i2c%d\n", busnum);
		return -1;
	}

	client = i2c_new_device(adap, &axp_regulator);
	if (!client) {
		pr_err("failed to register pmu to i2c%d\n", busnum);
		return -1;
	}

	i2c_put_adapter(adap);

	printk( "%s %d\n", __FUNCTION__, __LINE__ );

#if 0
	// set LDO2 -> 3.3V : DAC Digital
	regulator = regulator_get(0, "ldo2");
	if (!regulator)
	{
		printk("get ldo2 failed!\n");
		return -1;
	}

	regulator_set_voltage(regulator, 3300000, 3300000);

	regulator_enable(regulator);

	regulator_put(regulator);
#endif

	return 0;
}
subsys_initcall_sync(pmu_dev_init);
