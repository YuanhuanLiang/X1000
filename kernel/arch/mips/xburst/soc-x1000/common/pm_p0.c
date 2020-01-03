/*
 * linux/arch/mips/xburst/soc-xxx/common/pm_p0.c
 *
 *  X1000 Power Management Routines
 *  Copyright (C) 2006 - 2012 Ingenic Semiconductor Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#include <linux/init.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/delay.h>
#include <asm/fpu.h>
#include <linux/syscore_ops.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/notifier.h>
#include <asm/cacheops.h>
#include <soc/cache.h>
#include <asm/r4kcache.h>
#include <soc/base.h>
#include <soc/cpm.h>
#include <soc/tcu.h>
#include <soc/gpio.h>
#include <soc/ddr.h>
#include <tcsm.h>
#include <smp_cp0.h>
#include <jz_notifier.h>
#include <soc/tcsm_layout.h>

#define SLEEP_LIB_TCSM          0xb3422000
#define SLEEP_LIB_SIZE          (8 * 1024)

struct sleep_lib_entry {
    void (*restore_context)(void);
    int (*enter_sleep)(int state);
};

extern void show_wakeup_sources(void);
extern void record_suspend_time(void);


static int x1000_pm_enter(suspend_state_t state) {
    struct sleep_lib_entry *entry = (void *)(SLEEP_LIB_TCSM);

    printk("enter sleep lib\n");
    record_suspend_time();

#ifdef CONFIG_JZ_USB_REMOTE_WKUP_PIN
    /*usb switch*/
    usb_remote_wkup_notifier_call_chain(USB_REMOTE_WKUP_PIN_SUSPEND);
#endif

    entry->enter_sleep(state);

#ifdef CONFIG_JZ_USB_REMOTE_WKUP_PIN
    usb_remote_wkup_notifier_call_chain(USB_REMOTE_WKUP_PIN_RESUME);
#endif

    show_wakeup_sources();
    printk("quit sleep lib\n");

    return 0;
}

/*
 * Initialize power interface
 */
struct platform_suspend_ops pm_ops = {
	.valid = suspend_valid_only_mem,
	.enter = x1000_pm_enter,
};


int __init x1000_pm_init(void)
{
	suspend_set_ops(&pm_ops);
	return 0;
}

arch_initcall(x1000_pm_init);
