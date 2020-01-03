#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
//#include <linux/default_backlight.h>
#include "../jz_fb_v13/jz_fb.h"
#include "jz_logo.h"
#include <linux/input.h>
#ifdef CONFIG_BATTERY_VOLTAGE_LIMIT
#define BATTERY_DEFAULT_MIN                 CONFIG_BATTERY_VOLTAGE_LIMIT
#else
#define BATTERY_DEFAULT_MIN                 3600
#endif

static struct gpio_keys_button * jz_logo_button = NULL;
static struct gpio_keys_platform_data * jz_logo_button_data = NULL;

static int power_key = -1;
static int charge_det = -1;
static int power_active = -1;
static int charge_active = -1;


#if CONFIG_LOGO_LINUX_CHARGE_CLUT224

static const struct linux_logo * charge_logo[] = {

 &logo_linux_charge_clut224,

};
#endif

static void show_charge_logo(struct fb_info * info)
{
#if CONFIG_LOGO_LINUX_CHARGE_CLUT224
   printk("show_charge_logo******\n");
   if (jz_fb_prepare_logo(info, &logo_linux_charge_clut224, FB_ROTATE_UR)) {
       fb_show_logo(info, FB_ROTATE_UR);
   }
   fb_blank(info, FB_BLANK_UNBLANK);
#endif


}


static void show_boot_logo(struct fb_info *info)
{
    //fb_blank(info, FB_BLANK_VSYNC_SUSPEND);

    if (fb_prepare_logo(info, FB_ROTATE_UR)) {
        /* Start display and show logo on boot */
        fb_show_logo(info, FB_ROTATE_UR);
    }

    fb_blank(info, FB_BLANK_UNBLANK);
}

static void show_low_battery_logo(struct fb_info *info)
{
#if CONFIG_LOGO_LINUX_CHARGE_CLUT224
    if (jz_fb_prepare_logo(info, &logo_linux_charge_clut224, FB_ROTATE_UR)) {
        fb_show_logo(info, FB_ROTATE_UR);
    }

    fb_blank(info, FB_BLANK_UNBLANK);
    msleep(2000);
    fb_blank(info, FB_BLANK_VSYNC_SUSPEND);
#endif
}

static int is_battery_low(void)
{
    struct class_dev_iter iter;
    struct device *dev;
    struct power_supply *psy;
    union power_supply_propval ret = { 0, };

    class_dev_iter_init(&iter, power_supply_class, NULL, NULL);
    while ((dev = class_dev_iter_next(&iter))) {
        psy = dev_get_drvdata(dev);
        if (psy->type == POWER_SUPPLY_TYPE_BATTERY) {
            if (!psy->get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &ret)) {
                break;
            }
        }
    }
    if (ret.intval < BATTERY_DEFAULT_MIN) {
        printk("battery capacity too low [%d mV].\n", ret.intval);
    }
    return (ret.intval < BATTERY_DEFAULT_MIN) ? 1 : 0;
}

static int is_battery_charging(void)
{
    struct class_dev_iter iter;
    struct device *dev;
    struct power_supply *psy;
    union power_supply_propval ret = { 0, };
    int is_charging = 0;

    class_dev_iter_init(&iter, power_supply_class, NULL, NULL);
    while ((dev = class_dev_iter_next(&iter))) {
        psy = dev_get_drvdata(dev);
        if (psy->type == POWER_SUPPLY_TYPE_BATTERY) {
            if (!psy->get_property(psy, POWER_SUPPLY_PROP_CHARGE_NOW, &ret)) {
                is_charging = ret.intval;
                break;
            }
        } else {
            if (!psy->get_property(psy, POWER_SUPPLY_PROP_ONLINE, &ret)) {
                is_charging = ret.intval;
                break;
            }
        }
    }

    return is_charging;
}
static int get_power_status(void)
{
    int power_status = 0;
    if (power_key == -1) {
         printk("power_key not prepare, power off\n");
         pm_power_off();
    }
    power_status = gpio_get_value(power_key);
    power_status ^= power_active;
    return power_status;
}
static int get_charge_status(void)
{
    int charge_status = 0;
    if (charge_det == -1) {
        printk("power_key not prepare, power off\n");
        pm_power_off();
    }
    charge_status = gpio_get_value(charge_det);
    charge_status ^= charge_active;
    return charge_status;
}

void show_logo(struct fb_info *info) {
    static int btn_press = 1;
    #define charge_logo_time  5000
    #define power_open_time  3000
    while (1) {
        if (is_battery_low()) {
                printk("battery is not charging ...\n");
                show_low_battery_logo(info);
                printk("power off now.\n");
                pm_power_off();
        }
        else {
            if (get_charge_status()) {
                if (is_battery_charging() && btn_press) {
                    printk("battery is charging ...\n");
                    show_charge_logo(info);
                    msleep(power_open_time);
                    if(get_power_status()){
                        show_boot_logo(info);
                        return;
                    }
                    msleep(charge_logo_time-power_open_time);
                    fb_blank(info, FB_BLANK_VSYNC_SUSPEND);
                }
                else if (!is_battery_charging()&& btn_press) {
                    //ÏÔÊ¾³äÂú½çÃæ
                    printk("battery is full\n");
                    show_charge_logo(info);
                    msleep(power_open_time);
                    if (get_power_status()) {
                        show_boot_logo(info);
                        return;
                    }
                    msleep(charge_logo_time-power_open_time);
                    fb_blank(info, FB_BLANK_VSYNC_SUSPEND);
                }

            }
            else{
                if (btn_press) {
                    msleep(power_open_time);
                    if (get_power_status()) {
                        show_boot_logo(info);
                        return;
                    }
                    else {
                        pm_power_off();
                    }
                }
            }

        }

    btn_press = get_power_status();

    }
}
EXPORT_SYMBOL_GPL(show_logo);

static int jzlogo_probe(struct platform_device *pdev) {
    jz_logo_button_data = pdev->dev.platform_data;
    int num = jz_logo_button_data->nbuttons;
    jz_logo_button = jz_logo_button_data->buttons;
    int i = 0;
    for (i = 0; i < num; i++) {
        if (jz_logo_button[i].code == KEY_POWER) {
            power_key = jz_logo_button[i].gpio;
            power_active = jz_logo_button[i].active_low;
            jzgpio_set_func((jz_logo_button[i].gpio) / 32, GPIO_INPUT_PULL,(jz_logo_button[i].gpio) % 32);
        }
        else if (jz_logo_button[i].code == KEY_BATTERY) {
            charge_det = jz_logo_button[i].gpio;
            charge_active = jz_logo_button[i].active_low;
            jzgpio_set_func((jz_logo_button[i].gpio) / 32, GPIO_INPUT_PULL,(jz_logo_button[i].gpio) % 32);
        }

    }
    return 0;
}

static struct platform_driver jzlogo_driver = {
    .probe = jzlogo_probe,
    .driver = {
        .name = "jz_logo",
        .owner  = THIS_MODULE,
    },
};

int jzlogo_init(void)
{
    platform_driver_register(&jzlogo_driver);
    return 0;
}

static void __exit jzlogo_exit(void)
{
    platform_driver_unregister(&jzlogo_driver);
}

arch_initcall_sync(jzlogo_init);

module_exit(jzlogo_exit);
MODULE_LICENSE("GPL");
