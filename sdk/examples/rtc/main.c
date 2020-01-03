#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/rtc.h>
#include <sys/time.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <rtc/rtc_manager.h>

#define LOG_TAG "test_rtc"

int main(int argc, char **argv)
{
    struct rtc_manager *rtc;
    struct rtc_time time;
    int retval = 0;
    int alarm_interval = 20;

    rtc = get_rtc_manager();

    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 1;
    time.tm_mon = 0;
    time.tm_year = 2016-1900;
    time.tm_isdst = 0;

    if (rtc->init() < 0) {
        LOGE("Failed to init rtc manager\n");
        return -1;
    }

    retval = rtc->get_hibernate_wakeup_status();
    if (retval < 0) {
        LOGE("Failed to get hibernate wakeup status\n");
        return -1;
    }

    LOGI("Hibernate wakeup status: 0x%x\n", retval);


    if(rtc->set_rtc(&time) < 0) {
        LOGE("rtc write error\n");
        return -1;
    }

    if(rtc->get_rtc(&time) < 0) {
        LOGE("rtc read error\n");
        return -1;
    }

    LOGE("Current RTC date time is %d-%d-%d, %02d:%02d:%02d.\n",
            time.tm_mday, time.tm_mon + 1, time.tm_year + 1900,
            time.tm_hour, time.tm_min, time.tm_sec);

    time.tm_sec += alarm_interval;
    if (time.tm_sec >= 60) {
        time.tm_sec %= 60;
        time.tm_min++;
    }

    if (time.tm_min == 60) {
        time.tm_min = 0;
        time.tm_hour++;
    }

    if (time.tm_hour == 24)
        time.tm_hour = 0;

    retval = rtc->set_bootup_alarm(&time);
    if (retval < 0) {
        LOGE("Failed to set bootup alarm\n");
        return -1;
    }

    rtc->deinit();

    return 0;
}
