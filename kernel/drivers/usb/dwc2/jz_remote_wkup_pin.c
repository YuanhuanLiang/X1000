#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/of.h>

#include <soc/base.h>
#include <linux/jz_dwc.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <soc/cpm.h>
#include <jz_notifier.h>
#include <soc/gpio.h>
#include "core.h"
#include "jz_remote_wkup_pin.h"

#ifdef CONFIG_JZ_USB_REMOTE_WKUP_PIN

extern void jz_usb_remote_wkup_pin_interrupt(struct dwc2 *dwc);
void usb_dwc2_remote_wkup_noirq(unsigned long action);

/* usb switch pin */
struct jzdwc_pin __attribute__((weak)) dwc2_switch_pin = {
    .num                    = -1,
    .enable_level           = -1,   /* suspend status level */
};

/* usb remote wake up pin */
struct jzdwc_pin __attribute__((weak)) dwc2_remote_wkup_pin = {
    .num                    = -1,
    .enable_level           = -1,
};

static struct dwc2 *dwc_remote_wkup;
static BLOCKING_NOTIFIER_HEAD(usb_remote_wkup_chain_head);

int register_usb_remote_wkup_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_register(&usb_remote_wkup_chain_head, nb);
}
EXPORT_SYMBOL_GPL(register_usb_remote_wkup_notifier);

int unregister_usb_remote_wkup_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_unregister(&usb_remote_wkup_chain_head, nb);
}
EXPORT_SYMBOL_GPL(unregister_usb_remote_wkup_notifier);

int usb_remote_wkup_notifier_call_chain(unsigned long val)
{
    int ret = blocking_notifier_call_chain(&usb_remote_wkup_chain_head, val, NULL);

    return notifier_to_errno(ret);
}

void usb_dwc2_remote_wkup_noirq(unsigned long action)
{
    unsigned int opcr;
    struct remote_wkup_pin *jz_remote_wkup_pin;
    jz_remote_wkup_pin = dwc_remote_wkup->jz_remote_wkup_pin;

    switch (action) {
    case USB_REMOTE_WKUP_PIN_SUSPEND:
        opcr = cpm_inl(CPM_OPCR) & (~(1<<7));
        cpm_outl(opcr,CPM_OPCR);
        gpio_direction_output(jz_remote_wkup_pin->switch_pin, !!jz_remote_wkup_pin->switch_pin_level);
        break;

    case USB_REMOTE_WKUP_PIN_RESUME:
        opcr = cpm_inl(CPM_OPCR) | (1<<7);
        cpm_outl(opcr,CPM_OPCR);
        gpio_direction_output(jz_remote_wkup_pin->switch_pin, !jz_remote_wkup_pin->switch_pin_level);
        cpm_clear_bit(3,CPM_CLKGR);/*otg clk gate*/
        udelay(100);
        jz_remote_wkup_pin->wkup_state= 1;
        jz_usb_remote_wkup_pin_interrupt(dwc_remote_wkup);
        jz_remote_wkup_pin->wkup_state= 0;
        break;
    }
}

static int usb_remote_wkup_pm_callback(struct notifier_block *nfb,unsigned long action,void *ignored)
{
    usb_dwc2_remote_wkup_noirq(action);

    return NOTIFY_OK;
}
static struct notifier_block usb_remote_wkup_pin_notifier = {
    .notifier_call  = usb_remote_wkup_pm_callback,
    .priority       = 0,
};

static irqreturn_t remote_wake_detect_irq(int irq, void* devid)
{
    /*
     * TODO
     */

    return IRQ_HANDLED;
}

#ifdef CONFIG_JZ_USB_SUSPEND_REPORT_KEY
int jz_usb_remote_wkup_report_event(void)
{
    struct remote_wkup_pin *jz_remote_wkup_pin;
    jz_remote_wkup_pin = dwc_remote_wkup->jz_remote_wkup_pin;

    if (jz_remote_wkup_pin->input_dev) {
        /* auto-repeat bypasses state updates */
        input_event(jz_remote_wkup_pin->input_dev, EV_KEY, KEY_SUSPEND, 2);
        input_sync(jz_remote_wkup_pin->input_dev);
    }

    return 0;
}
#endif

int jz_usb_remote_wkup_pin_init(struct dwc2 *dwc)
{
    struct remote_wkup_pin *jz_remote_wkup_pin;
    unsigned long pins;
    enum gpio_port port;
    int ret = 0;
    int irq_num;
    struct input_dev *input;

    jz_remote_wkup_pin = kzalloc(sizeof(struct remote_wkup_pin),GFP_KERNEL);
    if(!jz_remote_wkup_pin) {
        printk("%s: not enough memory!!\n",__func__);
        return -ENOMEM;
    }

    /*register ws*/
    jz_remote_wkup_pin->dwc2_ws             = wakeup_source_register("dwc2-suspend");
    jz_remote_wkup_pin->switch_pin          = dwc2_switch_pin.num;
    jz_remote_wkup_pin->switch_pin_level    = dwc2_switch_pin.enable_level;
    jz_remote_wkup_pin->irq_pin             = dwc2_remote_wkup_pin.num;
    jz_remote_wkup_pin->wkup_state = 0;

    if (!gpio_is_valid(jz_remote_wkup_pin->switch_pin)) {
        printk("USB Switch Pin is invalid.\n");
        kfree(jz_remote_wkup_pin);
        return -1;
    }

    if (!gpio_is_valid(jz_remote_wkup_pin->irq_pin)) {
        printk("USB WakeUP Pin is invalid.\n");
        kfree(jz_remote_wkup_pin);
        return -1;
    }

    dwc->jz_remote_wkup_pin = jz_remote_wkup_pin ;
    dwc_remote_wkup = dwc;

    register_usb_remote_wkup_notifier(&usb_remote_wkup_pin_notifier);
    __pm_stay_awake(jz_remote_wkup_pin->dwc2_ws);

    /* switch pin */
    gpio_request_one(jz_remote_wkup_pin->switch_pin, GPIOF_INIT_LOW, NULL); /*USB SEL INIT LOW*/
    if (ret < 0) {
        printk("Failed to request Switch Pin GPIO %d, error %d\n", jz_remote_wkup_pin->switch_pin, ret);
        return ret;
    }
    gpio_direction_output(jz_remote_wkup_pin->switch_pin, !jz_remote_wkup_pin->switch_pin_level);

    /* remote wakeup dete pin */
    gpio_request_one(jz_remote_wkup_pin->irq_pin, GPIOF_IN, NULL); /* DP INIT INPUT PULL*/
    if (ret < 0) {
        printk("Failed to request IRQ Pin GPIO %d, error %d\n", jz_remote_wkup_pin->irq_pin, ret);
        return ret;
    }

    /*
     *  IRQ pin Disable Pull
     */
    pins  = jz_remote_wkup_pin->irq_pin % 32;
    port  = jz_remote_wkup_pin->irq_pin / 32;

    jzgpio_ctrl_pull(port, 0, (1 << pins ));
    irq_num = gpio_to_irq(jz_remote_wkup_pin->irq_pin);
    ret = request_any_context_irq(irq_num,
            remote_wake_detect_irq,
            IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
            "remote wake irq",
            NULL);
    enable_irq_wake(irq_num);
    if (ret < 0) {
        printk("Failed to request remote wake up IRQ. error=%d\n",ret);
        return ret;
    }

#ifdef CONFIG_JZ_USB_SUSPEND_REPORT_KEY
    /*
     * Input
     */
    input = input_allocate_device();
    if (!input) {
        printk("failed to allocate input device\n");
        return -ENOMEM;
    }

    jz_remote_wkup_pin->input_dev = input;

    input_set_drvdata(input, jz_remote_wkup_pin);

    input->name = "USB-remote-wkup";
    input->id.bustype = BUS_HOST;
    input->id.vendor = 0x0002;
    input->id.product = 0x0002;
    input->id.version = 0x0001;

    __set_bit(EV_KEY, input->evbit);
    __set_bit(EV_SYN, input->evbit);
    input_set_capability(input, EV_KEY, KEY_SUSPEND);

    ret = input_register_device(input);
    if (ret) {
        printk("jz remote wkup pin: failed to register input device.\n");
        input_free_device(input);
        jz_remote_wkup_pin->input_dev = NULL;

        return -1;
    }
#endif
    return 0;
}

#endif
