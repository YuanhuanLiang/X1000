/*
 * Bluetooth bcm4330 rfkill power control via GPIO
 * Create by Ingenic cljiang <cljiang@ingenic.cn>
 *
 */

#ifndef _BT_RFKILL_H
#define _BT_RFKILL_H

#include <linux/rfkill.h>

struct bt_gpio {
	int bt_rst_n;
	int bt_reg_on;
	int bt_wake;
	int bt_int;
	int bt_uart_rts;
	int bt_int_flagreg;
	int bt_int_bit;
};

struct bt_rfkill_platform_data {
	struct bt_gpio gpio;

	struct rfkill *rfkill;  /* for driver only */
#ifdef CONFIG_PRODUCT_X1000_IDRIVER
    int power_state;
    int (*power_init)(void);
	int (*power_switch)(struct bt_rfkill_platform_data*, int);
#else
	void (*restore_pin_status)(int);
	void (*set_pin_status)(int);
#endif
};

#endif
