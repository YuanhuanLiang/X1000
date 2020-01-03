#include <linux/platform_device.h>
#include <linux/bcm_pm_core.h>
#include <linux/bt-rfkill.h>
#include <gpio.h>
#include <soc/gpio.h>
#include "board_base.h"
#include <linux/delay.h>
int power_init(void)
{
    int ret;

    //printk("%s, %d\n", __FUNCTION__, __LINE__);
    ret = gpio_request(GPIO_BT_REG_ON,"bt_reg_on");
    if(unlikely(ret)){
        return ret;
    }
    gpio_direction_output(GPIO_BT_REG_ON, 0);
    
//    ret = gpio_request(GPIO_BT_WAKE_HOST,"bt_wake_host");
//    if(unlikely(ret)){
//        return ret;
//    }
//    gpio_direction_output(GPIO_BT_WAKE_HOST,1);

//    ret = gpio_request(GPIO_HOST_WAKE_BT,"host_wake_bt");
//    if(unlikely(ret)){
//        return ret;
//    }
//    gpio_direction_output(GPIO_HOST_WAKE_BT,1);
    return 0;
}

static DEFINE_MUTEX(power_lock);
extern void rtc32k_enable(void);
extern void rtc32k_disable(void);
extern int get_ap6212_wlan_power_state(void);
static int ap6212_bt_power_state;
int get_ap6212_bt_power_state(void)
{
    return ap6212_bt_power_state;
}
int power_switch(struct bt_rfkill_platform_data* pdata, int enable)
{
    printk("%s, %d enable:%d\n", __FUNCTION__, __LINE__,enable);
	mutex_lock(&power_lock);
    if(enable){
        gpio_set_value(GPIO_BT_REG_ON, 1);
		rtc32k_enable();
    }else{
        gpio_set_value(GPIO_BT_REG_ON, 0);
        if(get_ap6212_wlan_power_state() == 0)
            rtc32k_disable();
    }
    msleep(100);
    pdata->power_state = enable;
    ap6212_bt_power_state = enable;
	mutex_unlock(&power_lock);
    return 0;
}

struct bt_rfkill_platform_data  ap6212_bt_data = {
    .power_init = power_init,
    .power_switch = power_switch,
};

struct platform_device bt_power_device = {
	.name = "bt_power",
	.id = -1,
	.dev = {
		.platform_data = &ap6212_bt_data,
	},
};
