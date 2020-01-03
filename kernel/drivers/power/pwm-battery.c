/*
 *  Copyright (C) 2017 Ingenic Semiconductor Co.,Ltd
 *
 *  PWM battery driver
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pwm.h>
#include <linux/hrtimer.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <linux/seq_file.h>
#include <linux/alarmtimer.h>
#include <linux/wakelock.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>
#include <linux/power/pwm_battery.h>

#define LOG_TAG "\e[0;91mBattery driver:\e[0m "

#define PWM_FREQ            (140 * 1000)
#define PWM_STABLE_TIME_US  200
#define PWM_SAMPLE_TIMES    3
#define PWM_DUTY_NS_STEP    30

/*
 * USB_DETE pin RC discharge too slowwwwwww
 */
#define DELAY_SAMPLE_USB_DETE_MS    (600)

struct pwm_battery {
    struct pwm_battery_platform_data *pdata;

    unsigned int usb_irq;
    unsigned int charger_irq;
    unsigned int battery_irq;

    int pwm_period;

    int care_usb;
    int care_charger;

    int usb_online;
    int charger_online;

    int status;
    int capacity_real;
    int capacity_show;

    int init_duty_ns;
    int duty_ns;
    int voltage;

    int next_scan_time;

    struct clk *pclk;

    struct mutex lock;

    struct power_supply usb;
    struct power_supply charger;
    struct power_supply battery;

    struct hrtimer timer;
    struct completion completion;

    struct notifier_block pm_nb;

    struct workqueue_struct *workqueue;

    struct timer_list charger_debounce_timer;

    struct delayed_work usb_work;
    struct delayed_work charger_work;
    struct delayed_work external_power_work;
    struct delayed_work battery_work;
    struct delayed_work resume_work;

    struct pwm_device *pwm;

    struct alarm alarm;
    struct wake_lock work_wake_lock;

    __kernel_time_t resume_time;
    __kernel_time_t suspend_time;
    __kernel_time_t last_update_time;

    struct proc_dir_entry* battery_proc;

    struct device* dev;
};

extern const int pwm_battery_cv_discharging[101];
extern const int pwm_battery_cv_charging[101];

static char* status_dbg(int status)
{
    switch (status) {
    case POWER_SUPPLY_STATUS_UNKNOWN:
        return "UNKNOWN";
    case POWER_SUPPLY_STATUS_CHARGING:
        return "CHARGING";
    case POWER_SUPPLY_STATUS_DISCHARGING:
        return "DISCHARGING";
    case POWER_SUPPLY_STATUS_NOT_CHARGING:
        return "NOT_CHARGING";
    case POWER_SUPPLY_STATUS_FULL:
        return "FULL";
    default:
        return "ERROR";
    }
}

static enum power_supply_property pwm_battery_usb_props[] = {
        POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property pwm_battery_charger_props[] = {
        POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property pwm_battery_voltage_props[] = {
        POWER_SUPPLY_PROP_STATUS,
        POWER_SUPPLY_PROP_TECHNOLOGY,
        POWER_SUPPLY_PROP_HEALTH,
        POWER_SUPPLY_PROP_CAPACITY,
        POWER_SUPPLY_PROP_VOLTAGE_NOW,
        POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
        POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
        POWER_SUPPLY_PROP_PRESENT,
};

static inline struct pwm_battery *usb_to_pwm_battery(struct power_supply *psy)
{
    return container_of(psy, struct pwm_battery, usb);
}

static inline struct pwm_battery *charger_to_pwm_battery(struct power_supply
        *psy) {
    return container_of(psy, struct pwm_battery, charger);
}

static inline struct pwm_battery* battery_to_pwm_battery(struct power_supply
        *psy) {
    return container_of(psy, struct pwm_battery, battery);
}

static int is_usb_online(struct pwm_battery_platform_data *pdata) {
    int temp;

    temp = gpio_get_value(pdata->gpio_usb);
    temp ^= pdata->gpio_usb_active_low;

    return temp;
}

static int is_charger_online(struct pwm_battery_platform_data *pdata) {
    int temp;

    temp = gpio_get_value(pdata->gpio_charger);
    temp ^= pdata->gpio_charger_active_low;

    return temp;
}

static int pwm_battery_usb_get_property(struct power_supply *psy,
        enum power_supply_property prop,
        union power_supply_propval *val) {
    struct pwm_battery *pwm_battery = usb_to_pwm_battery(psy);

    switch(prop) {
    case POWER_SUPPLY_PROP_ONLINE:
        val->intval = pwm_battery->usb_online;
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int pwm_battery_charger_get_property(struct power_supply *psy,
        enum power_supply_property prop,
        union power_supply_propval *val) {
    struct pwm_battery *pwm_battery = charger_to_pwm_battery(psy);

    switch(prop) {
    case POWER_SUPPLY_PROP_ONLINE:
        val->intval = pwm_battery->charger_online;
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static int pwm_battery_get_property(struct power_supply *psy,
        enum power_supply_property prop,
        union power_supply_propval *val) {
    struct pwm_battery *pwm_battery = battery_to_pwm_battery(psy);

    switch (prop) {
    case POWER_SUPPLY_PROP_STATUS:
        if ((pwm_battery->status == POWER_SUPPLY_STATUS_FULL)
                && (pwm_battery->capacity_show != 100))
            val->intval = POWER_SUPPLY_STATUS_CHARGING;
        else
            val->intval = pwm_battery->status;
        break;

    case POWER_SUPPLY_PROP_TECHNOLOGY:
        val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
        break;

    case POWER_SUPPLY_PROP_HEALTH:
        val->intval = POWER_SUPPLY_HEALTH_GOOD;
        break;

    case POWER_SUPPLY_PROP_CAPACITY:
        if ((pwm_battery->status == POWER_SUPPLY_STATUS_CHARGING)
                && (pwm_battery->capacity_show == 0))
            val->intval = 1;
        else
            val->intval = pwm_battery->capacity_show;
        break;

    case POWER_SUPPLY_PROP_VOLTAGE_NOW:
        val->intval = pwm_battery->voltage;
        break;

    case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
        val->intval = pwm_battery_cv_discharging[100];
        break;

    case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
        val->intval = pwm_battery_cv_discharging[0];
        break;

    case POWER_SUPPLY_PROP_PRESENT:
        val->intval = 1;
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static int is_charging(int status) {
    switch (status) {
    case POWER_SUPPLY_STATUS_CHARGING:
    case POWER_SUPPLY_STATUS_FULL:
        return 1;
    default:
        return 0;
    }
}

static enum hrtimer_restart pwm_wait_timeout(struct hrtimer * timer) {
    struct pwm_battery *pwm_battery = container_of(timer, struct pwm_battery, timer);

    complete(&pwm_battery->completion);

    return HRTIMER_NORESTART;
}

__attribute__((unused)) inline static void pwm_battery_restart_timer(struct pwm_battery* pwm_battery) {
    ktime_t ktime = ktime_set(PWM_STABLE_TIME_US / 1000000,
            (PWM_STABLE_TIME_US % 1000000) * 1000 );

    hrtimer_start(&pwm_battery->timer, ktime, HRTIMER_MODE_REL);
}

inline static void set_pwm_duty(struct pwm_battery* pwm_battery, int duty_ns) {
    if (duty_ns > pwm_battery->pwm_period || duty_ns < 0)
        return;

    pr_debug("period=%d, duty_ns=%d\n", pwm_battery->pwm_period, duty_ns);
    pwm_config(pwm_battery->pwm, duty_ns, pwm_battery->pwm_period);

    if (duty_ns == 0)
        pwm_disable(pwm_battery->pwm);
    else
        pwm_enable(pwm_battery->pwm);
}

static int get_battery_voltage(struct pwm_battery* pwm_battery) {
    struct pwm_battery_platform_data* pdata = pwm_battery->pdata;

    int i;
    int voltage[PWM_SAMPLE_TIMES];
    unsigned int duty_total = 0;
    int voltage_av = 0;
    unsigned int voltage_total = 0;
    int voltage_min = INT_MAX;
    int voltage_max = 0;
    int temp_battery_total = pdata->battery_ref_resistor1 + pdata->battery_ref_resistor2;
    int temp_battery_base = pdata->battery_ref_resistor2;

    memset((char*)voltage, 0 ,sizeof(int) * PWM_SAMPLE_TIMES);

    for (i = 0; i < PWM_SAMPLE_TIMES; i++) {
        int duty_ns = pwm_battery->init_duty_ns;

        mutex_lock(&pwm_battery->lock);
        INIT_COMPLETION(pwm_battery->completion);

        do {
            if (duty_ns >= pwm_battery->pwm_period)
                break;
            duty_ns += PWM_DUTY_NS_STEP;

            set_pwm_duty(pwm_battery, duty_ns);

            //pwm_battery_restart_timer(pwm_battery);
            //wait_for_completion_interruptible_timeout(&pwm_battery->completion, HZ);

            udelay(PWM_STABLE_TIME_US);

        } while(!(gpio_get_value(pdata->gpio_op) ^ pdata->gpio_op_active_low));

        mutex_unlock(&pwm_battery->lock);

        voltage[i] = pdata->pwm_ref_voltage * (duty_ns - PWM_DUTY_NS_STEP) / temp_battery_base
                * temp_battery_total / pwm_battery->pwm_period;

        voltage_total += voltage[i];

        pr_debug("voltage[%d]=%d\n", i, voltage[i]);

        if (voltage[i] < voltage_min)
            voltage_min = voltage[i];
        if (voltage[i] > voltage_max)
            voltage_max = voltage[i];

        duty_total += (duty_ns - PWM_DUTY_NS_STEP);
    }

    /*
     * disable pwm out to save power
     */
    set_pwm_duty(pwm_battery, 0);

    if (PWM_SAMPLE_TIMES > 2)
        voltage_av = (voltage_total - voltage_min - voltage_max) /
        (PWM_SAMPLE_TIMES - 2);
    else
        voltage_av = voltage_total;

#ifdef CONFIG_JZ_PWM
    /*
     * Note:
     *      Ingenic PWM special
     */
    if (PWM_FREQ > 100000)
        voltage_av = voltage_av * 998 / 1000;
#endif

    return voltage_av;
}

static int lookup_capacity(int charging, int volt) {
    int i;
    const int *cv;

    if (charging)
        cv = pwm_battery_cv_charging;
    else
        cv = pwm_battery_cv_discharging;

    for (i = 100; i > 0; --i) {
        if (volt >= cv[i])
            return i;
    }

    return 0;
}

static void pwm_battery_next_scantime_falling(struct pwm_battery* pwm_battery) {
    if (pwm_battery->capacity_show >= 90) {
        pwm_battery->next_scan_time = 120;
    } else if (pwm_battery->capacity_show >= 15) {
        pwm_battery->next_scan_time = 60;
    } else if (pwm_battery->capacity_show >= 8) {
        pwm_battery->next_scan_time = 30;
    } else {
        pwm_battery->next_scan_time = 10;
    }

    if (pwm_battery->capacity_real < pwm_battery->capacity_show - 10)
        pwm_battery->next_scan_time = 30;
}

static void pwm_battery_capacity_falling(struct pwm_battery* pwm_battery) {
    pwm_battery_next_scantime_falling(pwm_battery);

    if (pwm_battery->capacity_real < pwm_battery->capacity_show)
        if (pwm_battery->capacity_show != 0)
            pwm_battery->capacity_show--;
}

static void pwm_battery_next_scantime_rising(struct pwm_battery* pwm_battery) {
    pwm_battery->next_scan_time = 40;

    if (pwm_battery->capacity_show >= 100) {
        pwm_battery->next_scan_time = 60;
        pwm_battery->capacity_show = 99;
        return;
    } else if (pwm_battery->capacity_show == 99) {
        pwm_battery->next_scan_time = 60;
        return;
    }

    if (pwm_battery->capacity_real > pwm_battery->capacity_show + 5)
        pwm_battery->next_scan_time = 50;
    if (pwm_battery->capacity_real > pwm_battery->capacity_show + 10)
        pwm_battery->next_scan_time = 30;
}

static void pwm_battery_capacity_rising(struct pwm_battery* pwm_battery) {
    pwm_battery_next_scantime_rising(pwm_battery);

    if (pwm_battery->capacity_real > pwm_battery->capacity_show)
        if (pwm_battery->capacity_show != 100)
            pwm_battery->capacity_show++;
}

static void pwm_battery_capacity_full(struct pwm_battery* pwm_battery) {
    int old_status = pwm_battery->status;

    if (pwm_battery->capacity_show >= 99) {
        pwm_battery->capacity_show = 100;
        pwm_battery->status = POWER_SUPPLY_STATUS_FULL;
        pwm_battery->next_scan_time = 5 * 60;
    } else {
        pwm_battery->next_scan_time = 45;
        pwm_battery->capacity_show++;
        if (old_status == POWER_SUPPLY_STATUS_FULL) {
            pwm_battery->status = POWER_SUPPLY_STATUS_CHARGING;
            pr_info(LOG_TAG "status force changed from %s to %s,"
            " because cap show not 100!\n",
            status_dbg(old_status), status_dbg(pwm_battery->status));
        }
    }
}

static void external_power_changed_work(struct work_struct* work) {
    struct pwm_battery* pwm_battery = container_of(work, struct pwm_battery,
            external_power_work.work);

    struct pwm_battery_platform_data* pdata = pwm_battery->pdata;
    int status = 0;

    if (pwm_battery->care_charger) {

        if (is_charger_online(pdata)) {
            status = POWER_SUPPLY_STATUS_CHARGING;

        } else {
            if (pwm_battery->care_usb) {
                if (is_usb_online(pdata))
                    status = POWER_SUPPLY_STATUS_FULL;
                else
                    status = POWER_SUPPLY_STATUS_NOT_CHARGING;
            } else
                status = POWER_SUPPLY_STATUS_DISCHARGING;
        }

    } else {
        if (pwm_battery->care_usb) {
            if (is_usb_online(pdata))
                status = POWER_SUPPLY_STATUS_CHARGING;
            else
                status = POWER_SUPPLY_STATUS_DISCHARGING;
        } else
            status = POWER_SUPPLY_STATUS_DISCHARGING;
    }

    if (status != pwm_battery->status) {
        pr_info(LOG_TAG "Battery status %s changed to %s\n",
                status_dbg(pwm_battery->status), status_dbg(status));
        pwm_battery->status = status;
    }

    cancel_delayed_work(&pwm_battery->battery_work);
    queue_work(pwm_battery->workqueue, &pwm_battery->battery_work.work);
}

static void handle_battery_state_changed(struct pwm_battery* pwm_battery) {
    static int virgin = 1;
    int status;

    pwm_battery->voltage = get_battery_voltage(pwm_battery);
    if (virgin) {
        virgin = 0;

        if (pwm_battery->status != POWER_SUPPLY_STATUS_FULL) {
            pwm_battery->capacity_real = lookup_capacity(
                    is_charging(pwm_battery->status), pwm_battery->voltage);
            pwm_battery->capacity_show = pwm_battery->capacity_real;
        } else {
            pwm_battery->capacity_show = pwm_battery->capacity_real = 100;
        }

    } else {
        pwm_battery->capacity_real = lookup_capacity(
                is_charging(pwm_battery->status), pwm_battery->voltage);
    }

    /*
     * For board don't have charger status detect and usb insert detect
     */
    if (!pwm_battery->care_usb && !pwm_battery->care_charger) {
        pwm_battery->capacity_show = pwm_battery->capacity_real;
        pwm_battery->next_scan_time = 120;
        goto out;
    }

    switch (pwm_battery->status) {
    case POWER_SUPPLY_STATUS_CHARGING:
        pwm_battery_capacity_rising(pwm_battery);
        break;

    case POWER_SUPPLY_STATUS_FULL:
        pwm_battery_capacity_full(pwm_battery);
        break;

    case POWER_SUPPLY_STATUS_DISCHARGING:
    case POWER_SUPPLY_STATUS_NOT_CHARGING:
        pwm_battery_capacity_falling(pwm_battery);
        break;

    case POWER_SUPPLY_STATUS_UNKNOWN:
        pwm_battery->next_scan_time = 60;
        break;
    }

    /*
     * For board don't have charger status detect
     */
    if(!pwm_battery->care_charger) {
        status = pwm_battery->status;
        if (status == POWER_SUPPLY_STATUS_CHARGING) {
            if (pwm_battery->capacity_show == 100) {
                status = POWER_SUPPLY_STATUS_FULL;
                pr_info(LOG_TAG "Battery status %s changed to %s\n",
                        status_dbg(pwm_battery->status), status_dbg(status));
                pwm_battery->status = status;
                pwm_battery->next_scan_time = 5 * 60;
            }
        }
    }

out:
    pr_info(LOG_TAG "%s, cap real: %d, cap show: %d, volt: %dmV, next %ds\n",
            status_dbg(pwm_battery->status), pwm_battery->capacity_real,
            pwm_battery->capacity_show, pwm_battery->voltage,
            pwm_battery->next_scan_time);

    power_supply_changed(&pwm_battery->battery);

    queue_delayed_work(pwm_battery->workqueue, &pwm_battery->battery_work,
            pwm_battery->next_scan_time * HZ);

    //queue_delayed_work(pwm_battery->workqueue, &pwm_battery->battery_work,
    //        2 * HZ);
}

static void pwm_battery_work(struct work_struct* work) {
    struct pwm_battery* pwm_battery;

    pwm_battery = container_of(work, struct pwm_battery, battery_work.work);

    handle_battery_state_changed(pwm_battery);
}

/* DWC2 */
#if defined(CONFIG_USB_DWC2_DUAL_ROLE) || defined(CONFIG_USB_DWC2_DEVICE_ONLY)
extern void dwc2_gadget_plug_change(int plugin);
#endif
static void usb_detect_work(struct work_struct *work) {
    struct pwm_battery *pwm_battery = container_of(work, struct pwm_battery,
            usb_work.work);
    struct pwm_battery_platform_data* pdata = pwm_battery->pdata;

    int online = is_usb_online(pdata);

    if (pwm_battery->usb_online != online) {
        /*
         * callbakc change otg insert status
         */
#if defined(CONFIG_USB_DWC2_DUAL_ROLE) || defined(CONFIG_USB_DWC2_DEVICE_ONLY)
        dwc2_gadget_plug_change(online);
#endif
        pr_info(LOG_TAG "USB status: %s\n", online ? "online" : "offline");

        pwm_battery->usb_online = online;

        power_supply_changed(&pwm_battery->usb);

        if (!delayed_work_pending(&pwm_battery->external_power_work))
            schedule_delayed_work(&pwm_battery->external_power_work,
                    msecs_to_jiffies(100));
    }

    enable_irq(pwm_battery->usb_irq);
}

static irqreturn_t usb_detect_irq(int irq, void *devid) {
    struct power_supply *usb = (struct power_supply*)devid;
    struct pwm_battery *pwm_battery = usb_to_pwm_battery(usb);

    disable_irq_nosync(pwm_battery->usb_irq);

    schedule_delayed_work(&pwm_battery->usb_work, msecs_to_jiffies(300));

    return IRQ_HANDLED;
}

static void charger_detect_work(struct work_struct* work) {
    struct pwm_battery *pwm_battery = container_of(work, struct pwm_battery,
            charger_work.work);
    struct pwm_battery_platform_data* pdata = pwm_battery->pdata;

    int online = is_charger_online(pdata);

    if (pwm_battery->charger_online != online) {

        pr_info(LOG_TAG "Charger status: %s\n", online ? "online" : "offline");

        pwm_battery->charger_online = online;

        power_supply_changed(&pwm_battery->charger);

        if (!delayed_work_pending(&pwm_battery->external_power_work))
            schedule_delayed_work(&pwm_battery->external_power_work,
                    msecs_to_jiffies(DELAY_SAMPLE_USB_DETE_MS));
    }

    enable_irq(pwm_battery->charger_irq);
}

/*
 * debounce charger status when system load too heavy, otherwise charger status
 * will unstable
 */
static void charger_debounce_timer_work(unsigned long data) {
    struct pwm_battery* pwm_battery = (struct pwm_battery *)data;
    struct pwm_battery_platform_data *pdata = pwm_battery->pdata;
    int i;

    int value = gpio_get_value(pdata->gpio_charger);

    for (i = 0; i < pdata->charger_debounce; i++) {
        mdelay(1);
        if (gpio_get_value(pdata->gpio_charger) != value)
            break;
    }

    if (i == pdata->charger_debounce)
        schedule_delayed_work(&pwm_battery->charger_work,
            msecs_to_jiffies(300));
    else
        pr_info(LOG_TAG "System load too heavy, skip charger status change!\n");
}

static irqreturn_t charger_detect_irq(int irq, void* devid) {
    struct power_supply *charger = (struct power_supply*)devid;
    struct pwm_battery *pwm_battery = charger_to_pwm_battery(charger);
    struct pwm_battery_platform_data *pdata = pwm_battery->pdata;

    disable_irq_nosync(pwm_battery->charger_irq);

    if (pdata->charger_debounce)
        mod_timer(&pwm_battery->charger_debounce_timer, jiffies);
    else
        schedule_delayed_work(&pwm_battery->charger_work,
            msecs_to_jiffies(300));

    return IRQ_HANDLED;
}

static int pwm_battery_init_pwm(struct pwm_battery* pwm_battery) {
    struct device* dev = pwm_battery->dev;
    const struct pwm_battery_platform_data* pdata = dev->platform_data;
    int error = 0;

    pwm_battery->pwm = pwm_request(pdata->pwm_id, "pwm_battery");
    if (IS_ERR(pwm_battery->pwm)) {
        error = PTR_ERR(pwm_battery->pwm);
        dev_err(dev, "Unable to request PWM %d\n", pdata->pwm_id);
        return error;
    }

    pwm_battery->pwm->active_level = !pdata->pwm_active_low;

    return 0;
}

static int pwm_battery_init_gpio(struct pwm_battery* pwm_battery) {
    struct device* dev = pwm_battery->dev;
    const struct pwm_battery_platform_data* pdata = dev->platform_data;
    int error = 0;
    unsigned int usb_irq;
    unsigned int charger_irq;

    if (gpio_is_valid(pdata->gpio_power)) {
        error = gpio_request(pdata->gpio_power, dev_name(dev));
        if (error) {
            dev_err(dev, "Failed to request power gpio: %d\n", error);
            return error;
        }

        error = gpio_direction_output(pdata->gpio_power, !(pdata->gpio_power_active_low));
        if (error) {
            dev_err(dev, "Failed to set power gpio output: %d\n", error);
            gpio_free(pdata->gpio_power);
            return error;
        }
    }

    if (!gpio_is_valid(pdata->gpio_op)) {
        dev_err(dev, "Invalid op out pin\n");
        return -ENODEV;
    } else {
        error = gpio_request(pdata->gpio_op, dev_name(dev));
        if (error) {
            dev_err(dev, "Failed to request OP out gpio: %d\n", error);
            goto fail_request_op;
        }

        error = gpio_direction_input(pdata->gpio_op);
        if (error) {
            dev_err(dev, "Failed to set OP out to input: %d\n", error);
            goto fail_set_op;
        }
    }

    if (!gpio_is_valid(pdata->gpio_usb)) {
        dev_info(dev, "Invalid USB detect pin,not care USB instert status.\n");
        pwm_battery->care_usb = 0;
    } else {
        error = gpio_request(pdata->gpio_usb, dev_name(dev));
        if (error) {
            dev_err(dev, "Failed to request USB detect gpio: %d\n", error);
            goto fail_request_usb;
        }

        error = gpio_direction_input(pdata->gpio_usb);
        if (error) {
            dev_err(dev, "Failed to set USB detect to input: %d\n", error);
            goto fail_set_usb;
        }

        usb_irq = gpio_to_irq(pdata->gpio_usb);
        error = request_any_context_irq(usb_irq, usb_detect_irq,
                IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
                dev_name(dev), &pwm_battery->usb);
        if (error) {
            dev_warn(dev, "Failed to request USB insert detect irq: %d\n",
                    error);
            goto fail_set_usb;

        } else {
            pwm_battery->usb_irq = usb_irq;
            enable_irq_wake(pwm_battery->usb_irq);
            disable_irq_nosync(pwm_battery->usb_irq);
        }

        INIT_DELAYED_WORK(&pwm_battery->usb_work, usb_detect_work);

        pwm_battery->care_usb = 1;
    }

    if (!gpio_is_valid(pdata->gpio_charger)) {
        dev_info(dev, "Invalid Charger status pin,not care Charger status.\n");
        pwm_battery->care_charger = 0;
    } else {
        error = gpio_request(pdata->gpio_charger, dev_name(dev));
        if (error) {
            dev_err(dev, "Failed to request Charger status detect gpio: %d\n",
                    error);
            goto fail_request_charger;
        }

        error = gpio_direction_input(pdata->gpio_charger);
        if (error) {
            dev_err(dev, "Failed to set Charger status detect to input: %d\n",
                    error);
            goto fail_set_charger;
        }

        charger_irq = gpio_to_irq(pdata->gpio_charger);
        error = request_any_context_irq(charger_irq, charger_detect_irq,
                IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
                dev_name(dev), &pwm_battery->charger);
        if (error) {
            dev_warn(dev, "Failed to request Charger status detect irq: %d\n",
                    error);
            goto fail_set_charger;

        } else {
            pwm_battery->charger_irq = charger_irq;
            enable_irq_wake(pwm_battery->charger_irq);
            disable_irq_nosync(pwm_battery->charger_irq);
        }

        INIT_DELAYED_WORK(&pwm_battery->charger_work, charger_detect_work);

        setup_timer(&pwm_battery->charger_debounce_timer,
                charger_debounce_timer_work, (unsigned long)pwm_battery);

        pwm_battery->care_charger = 1;
    }

    return 0;

fail_set_charger:
    gpio_free(pdata->gpio_charger);
fail_request_charger:
fail_set_usb:
    gpio_free(pdata->gpio_usb);
fail_request_usb:
fail_set_op:
    gpio_free(pdata->gpio_op);
fail_request_op:
    return error;
}

static ssize_t pwm_pattery_show_pwm_duty_ns(struct device* dev,
        struct device_attribute* attr, char* buf) {
    struct pwm_battery* pwm_battery = dev_get_drvdata(dev);


    if (strcmp(attr->attr.name, "pwm_duty_ns"))
        return -ENOENT;

    sprintf(buf, "%d\n", pwm_battery->duty_ns);

    return strlen(buf);
}

static ssize_t pwm_battery_store_pwm_duty_ns(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count) {
    int value = 0;

    if (strcmp(attr->attr.name, "pwm_duty_ns"))
        return -ENOENT;

    sscanf(buf, "%du", &value);

    return count;
}

static struct device_attribute pwm_battery_sysfs_attrs[] = {
        __ATTR(pwm_duty_ns, S_IRUGO|S_IWUSR, pwm_pattery_show_pwm_duty_ns,
                pwm_battery_store_pwm_duty_ns),
};

static int battery_capacity_open(struct inode* inode, struct file* file) {
    file->private_data = PDE_DATA(inode);

    return 0;
}

static int battery_capacity_close(struct inode* inode, struct file* file) {
    file->private_data = NULL;

    return 0;
}

static int battery_capacity_write(struct file *file, const char __user *buf,
        size_t count, loff_t *data) {
    int cap;
    char str[10];
    struct pwm_battery *pwm_battery = (struct pwm_battery*) file->private_data;;

    str[9] = '\0';

    if (copy_from_user(str, buf, min(count, 9UL)))
        return -EFAULT;
    cap = simple_strtol(str, 0, 10);

    pr_info(LOG_TAG "Set capacity from %d to %d\n", pwm_battery->capacity_show, cap);

    if ((pwm_battery->status == POWER_SUPPLY_STATUS_FULL) && (cap <= 85))
        pr_info(LOG_TAG "skip because status is FULL\n");
    else if (abs(pwm_battery->capacity_real - cap) >= 35)
        pr_info(LOG_TAG "skip because abs >= 35\n");
    else
        pwm_battery->capacity_show = cap;

    cancel_delayed_work(&pwm_battery->battery_work);
    queue_work(pwm_battery->workqueue, &pwm_battery->battery_work.work);

    return count;
}


static const struct file_operations capacity_proc_fops = {
    .owner = THIS_MODULE,
    .open = battery_capacity_open,
    .release = battery_capacity_close,
    .write = battery_capacity_write,
    .llseek = seq_lseek,
};

static int battery_status_show(struct seq_file *m, void *v) {
    struct pwm_battery *pwm_battery = m->private;;

    return seq_printf(m,
            "%s, cap real: %d, cap show: %d, volt: %dmV, next: %ds\n",
            status_dbg(pwm_battery->status), pwm_battery->capacity_real,
            pwm_battery->capacity_show, pwm_battery->voltage,
            pwm_battery->next_scan_time);
}

static int battery_status_open(struct inode* inode, struct file* file) {
    return single_open(file, battery_status_show, PDE_DATA(inode));
}

static const struct file_operations status_proc_fops = {
    .owner = THIS_MODULE,
    .open = battery_status_open,
    .release = seq_release,
    .llseek = seq_lseek,
    .read = seq_read,
};

static enum alarmtimer_restart wake_up_fun(struct alarm *alarm, ktime_t ktime)
{
    return ALARMTIMER_NORESTART;
}

static void pwm_battery_set_resume_time(struct pwm_battery* pwm_battery) {
    struct timespec alarm_time;
    unsigned long interval = 60 * 60;

    unsigned int sleep_current = pwm_battery->pdata->battery_info.sleep_current;
    unsigned int battery_max_cpt = pwm_battery->pdata->battery_info.battery_max_cpt;
    int capacity_show = pwm_battery->capacity_show;

    getnstimeofday(&alarm_time);
    pr_info(LOG_TAG "time now is %ld\n", alarm_time.tv_sec);

    if (sleep_current == 0) {
        interval = 60 * 60;

    } else if (capacity_show > 10) {
        interval = ((capacity_show - 10) * battery_max_cpt * 36) / sleep_current;

    } else if (capacity_show > 3) {
        interval = ((capacity_show - 3) * battery_max_cpt * 36) / sleep_current;

    } else if (capacity_show > 1) {
        interval = ((capacity_show - 1) * battery_max_cpt * 36) / sleep_current;

    } else if (capacity_show == 1) {
        interval = (battery_max_cpt * 36) / sleep_current;
    }

    if (interval == 0)
        interval = 60 * 60;

    alarm_time.tv_sec += interval;
    pr_info(LOG_TAG "set resume time %ld(delta %ld, %ldh %ldm %lds)\n",
            alarm_time.tv_sec, interval, interval / 3600, (interval / 60) % 60,
            interval % 60);

    alarm_start(&pwm_battery->alarm, timespec_to_ktime(alarm_time));
}

static void pwm_battery_resume_work(struct work_struct* work) {
    struct delayed_work *delayed_work =
            container_of(work, struct delayed_work, work);
    struct pwm_battery *pwm_battery =
            container_of(delayed_work, struct pwm_battery, resume_work);
    struct timeval battery_time;
    unsigned long time_delta;
    unsigned int next_scan_time;
    unsigned int battery_is_updated = 1;

    pr_info(LOG_TAG "%s\n", __FUNCTION__);

    pwm_battery->voltage = get_battery_voltage(pwm_battery);

    pwm_battery->capacity_real = lookup_capacity(is_charging(pwm_battery->status),
            pwm_battery->voltage);

    do_gettimeofday(&battery_time);
    pwm_battery->resume_time = battery_time.tv_sec;
    pr_info(LOG_TAG "resume_time is %ld\n", battery_time.tv_sec);
    time_delta = pwm_battery->resume_time - pwm_battery->suspend_time;
    pr_info(LOG_TAG "time_delta is %ld, %ldh %ldm %lds\n",
            time_delta, time_delta/3600, (time_delta/60)%60, time_delta%60);

    next_scan_time = pwm_battery->next_scan_time;
    pwm_battery->next_scan_time = 15;

    if (pwm_battery->last_update_time == 0)
        pwm_battery->last_update_time = pwm_battery->resume_time;

    pr_info(LOG_TAG "suspend: %ld last:%ld  next_scan:%d delta:%ld\n",
            pwm_battery->suspend_time, pwm_battery->last_update_time,
            next_scan_time, pwm_battery->suspend_time - pwm_battery->last_update_time);

    if (pwm_battery->status == POWER_SUPPLY_STATUS_FULL) {
        pwm_battery->capacity_show = pwm_battery->capacity_real = 100;

    } else if (pwm_battery->status == POWER_SUPPLY_STATUS_CHARGING) {

        if ((pwm_battery->capacity_real > pwm_battery->capacity_show)
                && (time_delta > 10 * 60)) {
            pwm_battery->capacity_show = pwm_battery->capacity_real;

        } else if (time_delta > 3 * 60) {
            pwm_battery_capacity_rising(pwm_battery);

        } else if ((pwm_battery->suspend_time - pwm_battery->last_update_time)
                > next_scan_time) {
            pwm_battery_capacity_rising(pwm_battery);

        } else {
            pwm_battery_next_scantime_rising(pwm_battery);
            battery_is_updated = 0;
        }

    } else { /* DISCHARGING || NOT_CHARGING */

        if ((pwm_battery->capacity_real < pwm_battery->capacity_show)
                && (time_delta > 10 * 60)) {
            pwm_battery->capacity_show = pwm_battery->capacity_real;

        } else if (time_delta > 3 * 60) {
            pwm_battery_capacity_falling(pwm_battery);

        } else if ((pwm_battery->suspend_time - pwm_battery->last_update_time)
                > next_scan_time) {
            pwm_battery_capacity_falling(pwm_battery);

        } else {
            pwm_battery_next_scantime_falling(pwm_battery);
            /* get the battery info then update it */
            //battery_is_updated = 1;
        }
    }

    if (battery_is_updated) {
        pwm_battery->last_update_time = pwm_battery->resume_time;
        power_supply_changed(&pwm_battery->battery);
    }

    pr_info(LOG_TAG "%s, cap real: %d, cap show: %d, volt: %dmV, next %ds\n",
            status_dbg(pwm_battery->status), pwm_battery->capacity_real,
            pwm_battery->capacity_show, pwm_battery->voltage,
            pwm_battery->next_scan_time);

    schedule_delayed_work(&pwm_battery->battery_work,
            pwm_battery->next_scan_time * HZ);

    pwm_battery_set_resume_time(pwm_battery);

    wake_unlock(&pwm_battery->work_wake_lock);
}

#ifdef CONFIG_PM
static int pwm_battery_notify(struct notifier_block *nb, unsigned long mode,
        void *_unused) {
    struct pwm_battery *pwm_battery = container_of(nb, struct pwm_battery,
                                                pm_nb);

    pr_info(LOG_TAG "%s\n", __FUNCTION__);

    switch (mode) {
    case PM_SUSPEND_PREPARE:
        pwm_battery_set_resume_time(pwm_battery);
        break;

    default:
        break;
    }

    return 0;
}
#endif

static int pwm_battery_probe(struct platform_device* pdev) {
    struct device *dev = &pdev->dev;
    struct pwm_battery_platform_data* pdata = dev->platform_data;
    struct pwm_battery *pwm_battery;
    struct proc_dir_entry *res_capacity;
    struct proc_dir_entry *res_status;
    unsigned long clk_rate = 0;
    unsigned int precision;
    int temp_battery_total = pdata->battery_ref_resistor1 + pdata->battery_ref_resistor2;
    int temp_battery_base = pdata->battery_ref_resistor2;
    int error = 0;
    int i;

    if (!pdata) {
        dev_err(&pdev->dev, "No platform data\n");
        return -EINVAL;
    }

    pwm_battery = devm_kzalloc(&pdev->dev, sizeof(*pwm_battery), GFP_KERNEL);
    if (!pwm_battery) {
        dev_err(&pdev->dev, "Failed to alloc driver structure\n");
        return -ENOMEM;
    }

    pwm_battery->dev = dev;
    pwm_battery->pdata = pdata;

    error = pwm_battery_init_pwm(pwm_battery);
    if (error < 0) {
        dev_err(&pdev->dev, "Failed to init pwm: %d\n", error);
        goto fail_init_pwm;

    }

    error = pwm_battery_init_gpio(pwm_battery);
    if (error < 0) {
        dev_err(&pdev->dev, "Failed to init gpio: %d\n", error);
        goto fail_init_gpio;
    }

    if (pwm_battery->care_usb) {
        pwm_battery->usb.name = "usb";
        pwm_battery->usb.type = POWER_SUPPLY_TYPE_USB;
        pwm_battery->usb.properties = pwm_battery_usb_props;
        pwm_battery->usb.num_properties = ARRAY_SIZE(pwm_battery_usb_props);
        pwm_battery->usb.get_property = pwm_battery_usb_get_property;

        error = power_supply_register(dev, &pwm_battery->usb);
        if (error) {
            dev_err(dev, "Failed to register usb power supply\n");
            goto fail_register_usb;
        }
    }

    if (pwm_battery->care_charger) {
        pwm_battery->charger.name = "charger";
        pwm_battery->charger.type = POWER_SUPPLY_TYPE_MAINS;
        pwm_battery->charger.properties = pwm_battery_charger_props;
        pwm_battery->charger.num_properties = ARRAY_SIZE(pwm_battery_charger_props);
        pwm_battery->charger.get_property = pwm_battery_charger_get_property;

        error = power_supply_register(dev, &pwm_battery->charger);
        if (error) {
            dev_err(dev, "Failed to register charger power supply\n");
            goto fail_register_charger;
        }
    }

    temp_battery_total = pdata->battery_ref_resistor1 + pdata->battery_ref_resistor2;
    temp_battery_base = pdata->battery_ref_resistor2;
    pwm_battery->battery.name = "battery";
    pwm_battery->battery.type = POWER_SUPPLY_TYPE_BATTERY;
    pwm_battery->battery.properties = pwm_battery_voltage_props;
    pwm_battery->battery.num_properties = ARRAY_SIZE(pwm_battery_voltage_props);
    pwm_battery->capacity_real = 100;
    pwm_battery->capacity_show = 100;
    pwm_battery->voltage = pdata->pwm_ref_voltage * temp_battery_total / temp_battery_base;
    pwm_battery->battery.get_property = pwm_battery_get_property;

    error = power_supply_register(dev, &pwm_battery->battery);
    if (error) {
        dev_err(dev, "Failed to register battery power supply\n");
        goto fail_register_battery;
    }

    pwm_battery->pclk = clk_get(&pdev->dev, "pclk");
    if (IS_ERR(pwm_battery->pclk)) {
        pr_err("%s %d,get pclk error\n",
                __func__, __LINE__);
        goto fail_register_battery;
    }
    clk_rate = clk_get_rate(pwm_battery->pclk);

    pwm_battery->status = POWER_SUPPLY_STATUS_NOT_CHARGING;
    pwm_battery->usb_online = -1;
    pwm_battery->charger_online = -1;
    pwm_battery->next_scan_time = 15;
    pwm_battery->pwm_period = 1000000000 / PWM_FREQ;

    precision = clk_rate / PWM_FREQ;
    if (precision < (2 << 8))
        pr_warn(LOG_TAG "!!!!! PWM precision %d less than 8-bits,"
                "result unreliable !!!!!\n",precision);

    pwm_battery->init_duty_ns = (pwm_battery->pwm_period * pwm_battery_cv_discharging[0]) /
                    (temp_battery_total * pwm_battery->pdata->pwm_ref_voltage / temp_battery_base);
    mutex_init(&pwm_battery->lock);
    init_completion(&pwm_battery->completion);

    alarm_init(&pwm_battery->alarm, ALARM_REALTIME, wake_up_fun);
    wake_lock_init(&pwm_battery->work_wake_lock, WAKE_LOCK_SUSPEND, "pwm_battery");

#ifdef CONFIG_PM
    pwm_battery->pm_nb.notifier_call = pwm_battery_notify;
    error = register_pm_notifier(&pwm_battery->pm_nb);
    if (error) {
        dev_err(dev, "Failed to register pm notifer\n");
        goto fail_pm;
    }
#endif

    for (i = 0; i < ARRAY_SIZE(pwm_battery_sysfs_attrs); i++) {
        error = device_create_file(dev, &pwm_battery_sysfs_attrs[i]);
        if (error) {
            dev_err(dev, "Failed to create sysfs interface: %d\n", error);
            break;
        }
    }

    if (error)
        goto fail_create_sysfs;

    hrtimer_init(&pwm_battery->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    pwm_battery->timer.function = pwm_wait_timeout;

    INIT_DELAYED_WORK(&pwm_battery->external_power_work, external_power_changed_work);
    INIT_DELAYED_WORK(&pwm_battery->resume_work, pwm_battery_resume_work);
    INIT_DELAYED_WORK(&pwm_battery->battery_work, pwm_battery_work);
    pwm_battery->workqueue = create_singlethread_workqueue("pwm-battery");
    queue_delayed_work(pwm_battery->workqueue, &pwm_battery->battery_work,
            5 * HZ);

    if (pwm_battery->care_usb)
        schedule_delayed_work(&pwm_battery->usb_work, 1 * HZ);
    if (pwm_battery->care_charger)
        schedule_delayed_work(&pwm_battery->charger_work, 1 * HZ);

    set_pwm_duty(pwm_battery, pwm_battery->init_duty_ns);

    pwm_battery->battery_proc = proc_mkdir("power_supply", 0);

    res_capacity =
            proc_create_data("capacity", 0200, pwm_battery->battery_proc,
                    &capacity_proc_fops, pwm_battery);
    if (!res_capacity) {
        dev_err(&pdev->dev, "Failed to Create proc capacity\n");
        goto fail_proc_capacity;
    }

    res_status = proc_create_data("status", 0444, pwm_battery->battery_proc,
            &status_proc_fops, pwm_battery);
    if (!res_status) {
        dev_err(&pdev->dev, "Failed to Create proc status\n");
        goto fail_proc_status;
    }

    platform_set_drvdata(pdev, pwm_battery);
    pwm_battery->voltage = get_battery_voltage(pwm_battery);
    pr_info(LOG_TAG "battery voltage is %dmv\n", pwm_battery->voltage);
    pr_info(LOG_TAG "pwm freq is %dHz\n", PWM_FREQ);
    pr_info(LOG_TAG "pwm precision is 1/%d\n", precision);
    pr_info(LOG_TAG "discharging max voltage is %d\n", pwm_battery_cv_discharging[100]);
    pr_info(LOG_TAG "discharging min voltage is %d\n", pwm_battery_cv_discharging[0]);
    pr_info(LOG_TAG "charging max voltage is %d\n", pwm_battery_cv_charging[100]);
    pr_info(LOG_TAG "charging min voltage is %d\n", pwm_battery_cv_charging[0]);
    pr_info(LOG_TAG "sleep_current is %d\n", pdata->battery_info.sleep_current);
    pr_info(LOG_TAG "battery_max_cpt is %d\n", pdata->battery_info.battery_max_cpt);

    pr_info(LOG_TAG "%s driver probe over!", dev_name(dev));

    return 0;

fail_proc_status:
    proc_remove(res_capacity);
fail_proc_capacity:
fail_create_sysfs:
    clk_put(pwm_battery->pclk);
    mutex_destroy(&pwm_battery->lock);

#ifdef CONFIG_PM
fail_pm:
    unregister_pm_notifier(&pwm_battery->pm_nb);
#endif

fail_register_battery:
    if (pwm_battery->care_charger)
        power_supply_unregister(&pwm_battery->charger);
fail_register_charger:
    if (pwm_battery->care_usb)
        power_supply_unregister(&pwm_battery->usb);
fail_register_usb:
    if (pdata->gpio_usb > 0)
        gpio_free(pdata->gpio_usb);
    if (pdata->gpio_charger > 0)
        gpio_free(pdata->gpio_charger);
fail_init_gpio:
    pwm_free(pwm_battery->pwm);
fail_init_pwm:
    devm_kfree(&pdev->dev, pwm_battery);

    return error;
}

static int pwm_battery_remove(struct platform_device* pdev) {
    struct pwm_battery *pwm_battery = platform_get_drvdata(pdev);
    struct pwm_battery_platform_data* pdata = pwm_battery->pdata;
    int i;

    for (i = 0; i < ARRAY_SIZE(pwm_battery_sysfs_attrs); i++)
        device_remove_file(&pdev->dev, &pwm_battery_sysfs_attrs[i]);

    if (pwm_battery->care_usb) {
        power_supply_unregister(&pwm_battery->usb);
        if (pwm_battery->usb_irq > 0)
            free_irq(pwm_battery->usb_irq, &pwm_battery->usb);
        gpio_free(pdata->gpio_usb);

        cancel_delayed_work(&pwm_battery->usb_work);
    }

    if (pwm_battery->care_charger) {
        power_supply_unregister(&pwm_battery->charger);
        if (pwm_battery->charger_irq > 0)
            free_irq(pwm_battery->charger_irq, &pwm_battery->charger);
        gpio_free(pdata->gpio_charger);

        cancel_delayed_work(&pwm_battery->charger_work);

        if (pdata->charger_debounce > 0)
            del_timer_sync(&pwm_battery->charger_debounce_timer);
    }

#ifdef CONFIG_PM
    unregister_pm_notifier(&pwm_battery->pm_nb);
#endif

    wake_lock_destroy(&pwm_battery->work_wake_lock);

    cancel_delayed_work(&pwm_battery->resume_work);
    cancel_delayed_work(&pwm_battery->battery_work);
    destroy_workqueue(pwm_battery->workqueue);

    proc_remove(pwm_battery->battery_proc);

    clk_put(pwm_battery->pclk);

    devm_kfree(&pdev->dev, pwm_battery);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int pwm_battery_resume(struct device *dev) {
    struct pwm_battery* pwm_battery = dev_get_drvdata(dev);
    struct pwm_battery_platform_data* pdata = pwm_battery->dev->platform_data;

    if (gpio_is_valid(pdata->gpio_power))
        gpio_direction_output(pdata->gpio_power, !(pdata->gpio_power_active_low));

    cancel_delayed_work(&pwm_battery->resume_work);

    wake_lock(&pwm_battery->work_wake_lock);

    schedule_delayed_work(&pwm_battery->resume_work, HZ / 5);

    return 0;
}

static int pwm_battery_suspend(struct device* dev) {
    struct pwm_battery* pwm_battery = dev_get_drvdata(dev);
    struct pwm_battery_platform_data* pdata = pwm_battery->dev->platform_data;
    struct timeval battery_time;

    pr_info(LOG_TAG "%s\n", __FUNCTION__);

    pr_info(LOG_TAG "%s, cap real: %d, cap show: %d, volt: %dmV\n",
            status_dbg(pwm_battery->status), pwm_battery->capacity_real,
            pwm_battery->capacity_show, pwm_battery->voltage);

    cancel_delayed_work_sync(&pwm_battery->resume_work);
    cancel_delayed_work_sync(&pwm_battery->battery_work);

    do_gettimeofday(&battery_time);
    pwm_battery->suspend_time = battery_time.tv_sec;

    pr_info(LOG_TAG "suspend_time is %ld\n", battery_time.tv_sec);

    if (gpio_is_valid(pdata->gpio_power))
        gpio_direction_output(pdata->gpio_power, pdata->gpio_power_active_low);
    return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(pwm_battery_pm_ops, pwm_battery_suspend, pwm_battery_resume);

static struct platform_driver pwm_battery_driver = {
    .probe = pwm_battery_probe,
    .remove = pwm_battery_remove,
    .driver = {
        .name = "pwm-battery",
        .owner = THIS_MODULE,
        .pm = &pwm_battery_pm_ops,
    },
};

module_platform_driver(pwm_battery_driver);

MODULE_AUTHOR("ZhangYanMing <yanming.zhang@inegnic.com>");
MODULE_DESCRIPTION("Driver for battery which report their capacity throuth pwm compare");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-battery");
