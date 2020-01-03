/*
 *  Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 *
 *  X1000 series bootloader for u-boot/rtos/linux
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com, jamincheung@126.com>
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

#include <common.h>

static inline void wait_write_ready(void) {
    int timeout = 0x100000;

    while (!(rtc_inl(RTC_RTCCR) & RTC_RTCCR_WRDY) && timeout--);
    if (timeout <= 0)
        uart_puts("RTC wait_write_ready timeout!\n");
}

void rtc_clk_src_to_ext(void) {
    /*
     * Set OPCR.ERCS of CPM to 1
     */
    uint32_t opcr = cpm_inl(CPM_OPCR);
    opcr |= OPCR_ERCS;
    cpm_outl(opcr, CPM_OPCR);

    /*
     * Set CLKGR.RTC of CPM to 0
     */
    uint32_t clkgr = cpm_inl(CPM_CLKGR);
    clkgr &= ~CPM_CLKGR_RTC;
    cpm_outl(clkgr, CPM_CLKGR);

    /*
     * Set RTCCR.SELEXC to 1
     */
    wait_write_ready();
    uint32_t rtccr = rtc_inl(RTC_RTCCR);
    rtccr |= RTC_RTCCR_SELEXC;
    rtc_outl(rtccr, RTC_RTCCR);

    /*
     * Wait two clock period of clock
     */
    udelay(10);

    opcr &= ~OPCR_ERCS;
    cpm_outl(opcr, CPM_OPCR);

    udelay(10);

    /*
     * Check RTCCR.SELEXC == 1
     */
    rtccr = rtc_inl(RTC_RTCCR);
    if (!(rtccr & RTC_RTCCR_SELEXC)) {
        hang_reason("rtc clock sel failed\n\n");
    }
}

#ifdef CONFIG_HIBERNATE

static int inline rtc_write_reg(int reg,int value)
{
    int timeout = 0x2000;

    while(!(rtc_inl(RTC_RTCCR) & RTC_RTCCR_WRDY) && timeout--);
    if(timeout <= 0) {
        printf("WARN:NO USE RTC!!!!!\n");
        return -1;
    }

    rtc_outl(0xa55a,(RTC_WENR));
    while(!(rtc_inl(RTC_RTCCR) & RTC_RTCCR_WRDY));
    while(!(rtc_inl(RTC_WENR) & RTC_WENR_WEN));
    while(!(rtc_inl(RTC_RTCCR) & RTC_RTCCR_WRDY));
    rtc_outl(value,(reg));
    while(!(rtc_inl(RTC_RTCCR) & RTC_RTCCR_WRDY));

    return 0;
}


void enter_hibernate(uint32_t wait_time)
{
    uint32_t rtc_rtccr;

    local_irq_disable();
    /* Set minimum wakeup_n pin low-level assertion time for wakeup: 1000ms */
    rtc_write_reg(RTC_HWFCR, HWFCR_WAIT_TIME(wait_time));

    /* Set reset pin low-level assertion time after wakeup: must  > 60ms */
    rtc_write_reg(RTC_HRCR, HRCR_WAIT_TIME(60));

    /* clear wakeup status register */
    rtc_write_reg(RTC_HWRSR, 0x0);

#ifdef CONFIG_HIBERNATE_ALM_WKUP
    rtc_write_reg(RTC_HWCR, 0x9);
#else
    rtc_write_reg(RTC_HWCR, 0x8);
#endif
    /* Put CPU to hibernate mode */
    rtc_write_reg(RTC_HCR, 0x1);
    /*poweroff the pmu*/
    //jz_notifier_call(NOTEFY_PROI_HIGH, JZ_POST_HIBERNATION, NULL);

    rtc_rtccr = rtc_inl(RTC_RTCCR);
    rtc_rtccr |= 0x1 << 0;
    rtc_write_reg(RTC_RTCCR,rtc_rtccr);

    mdelay(200);

    while(1)
        printf("%s:We should NOT come here.%08x\n",__func__, rtc_inl(RTC_HCR));
}
#endif