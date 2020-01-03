#include <linux/mmc/host.h>
#include <linux/fs.h>
#include <linux/wlan_plat.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <mach/jzmmc.h>
#include <linux/bcm_pm_core.h>

#include "board_base.h"

#define WLAN_SDIO_INDEX			1

#define RESET               		0
#define NORMAL              		1

struct wifi_data {
	struct wake_lock                wifi_wake_lock;
	struct regulator                *wifi_vbat;
	struct regulator                *wifi_vddio;
	int                             wifi_reset;
} bcm_ap6212_data;

/* static int wl_pw_en = 0; */

static int ap6212_wlan_power_state;

int get_ap6212_wlan_power_state(void)
{
    return ap6212_wlan_power_state;
}
extern void rtc32k_enable(void);
extern void rtc32k_disable(void);
extern int get_ap6212_bt_power_state(void);

int bcm_ap6212_wlan_init(void)
{
	static struct wake_lock	*wifi_wake_lock = &bcm_ap6212_data.wifi_wake_lock;
	int reset;

    if(bcm_ap6212_data.wifi_reset == 0)
    {
        reset = GPIO_WIFI_REG_ON;
        if (gpio_request(GPIO_WIFI_REG_ON, "wifi_reset")) {
            pr_err("ERROR: no wifi_reset pin available !!\n");
            goto err_put_vddio;
        } else {
            gpio_direction_output(reset, 1);
        }
        bcm_ap6212_data.wifi_reset = reset;

        wake_lock_init(wifi_wake_lock, WAKE_LOCK_SUSPEND, "wifi_wake_lock");
    }

	return 0;

err_put_vddio:
	return -EINVAL;
}

int bcm_wlan_power_on(int flag)
{
	static struct wake_lock	*wifi_wake_lock = &bcm_ap6212_data.wifi_wake_lock;
	int reset = bcm_ap6212_data.wifi_reset;
	if (wifi_wake_lock == NULL)
		pr_warn("%s: invalid wifi_wake_lock\n", __func__);
	else if (!gpio_is_valid(reset))
		pr_warn("%s: invalid reset\n", __func__);
	else
		goto start;
	return -ENODEV;
start:
	printk("wlan power on:%d\n", flag);
	rtc32k_enable();

	switch(flag) {
		case RESET:
			jzmmc_clk_ctrl(WLAN_SDIO_INDEX, 1);

			gpio_set_value(reset, 0);
			msleep(10);

			gpio_set_value(reset, 1);

			break;
		case NORMAL:

			gpio_set_value(reset, 0);
			msleep(10);

			gpio_set_value(reset, 1);

			jzmmc_manual_detect(WLAN_SDIO_INDEX, 1);

			break;
	}
    ap6212_wlan_power_state = 1;
	wake_lock(wifi_wake_lock);
	return 0;
}

int bcm_wlan_power_off(int flag)
{
	static struct wake_lock	*wifi_wake_lock = &bcm_ap6212_data.wifi_wake_lock;
	int reset = bcm_ap6212_data.wifi_reset;

	if (wifi_wake_lock == NULL)
		pr_warn("%s: invalid wifi_wake_lock\n", __func__);
	else if (!gpio_is_valid(reset))
		pr_warn("%s: invalid reset\n", __func__);
	else
		goto start;
	return -ENODEV;
start:
	printk("wlan power off:%d\n", flag);
	switch(flag) {
		case RESET:
			gpio_set_value(reset, 0);

			break;
		case NORMAL:
			gpio_set_value(reset, 0);

			break;
	}

    ap6212_wlan_power_state = 0;
	wake_unlock(wifi_wake_lock);
    if(get_ap6212_bt_power_state() == 0){
       printk("%s: %d\n", __func__, __LINE__);
       rtc32k_disable();
    }
	return 0;
}
EXPORT_SYMBOL(bcm_ap6212_wlan_init);
EXPORT_SYMBOL(bcm_wlan_power_on);
EXPORT_SYMBOL(bcm_wlan_power_off);
EXPORT_SYMBOL(get_ap6212_wlan_power_state);

