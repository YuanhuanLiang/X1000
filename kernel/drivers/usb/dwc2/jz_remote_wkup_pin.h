#ifndef _JZ_REMOTE_WKUP_PIN_H_
#define _JZ_REMOTE_WKUP_PIN_H_

#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/debugfs.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>


struct remote_wkup_pin{
    struct wakeup_source *dwc2_ws;
    unsigned int switch_pin;
    unsigned int switch_pin_level;
    unsigned int irq_pin;
    unsigned int wkup_state;
    struct input_dev *input_dev;
};

#endif /* _JZ_REMOTE_WKUP_PIN_H_ */
