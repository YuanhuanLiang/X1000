/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Thu 15 Dec 2016 04:05:47 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <watchdog/watchdog_manager.h>


#define LOG_TAG      "test_wdt"
#define WDT_TIMEOUT  3

/*
 * Function
 */

int main(int argc, char *argv[])
{
    int second = 0;
    struct watchdog_manager *wdt;

    /* 获取操作看门狗的句柄 */
    wdt = get_watchdog_manager();

    /* 初始化看门狗,并设定timeout */
    wdt->init(WDT_TIMEOUT);
    wdt->enable(); /* 使能 */

    while(1) {
        sleep(1);
        second++;

        LOGI("After %02d second...\n", second);

        if(second/2 < 3)
            wdt->reset();   /* 喂狗 */

        else if(second/2 == 3)
            wdt->disable(); /* 关闭看门狗 */

        else if(second/2 == 4) {
            wdt->init(5);   /* 重新设置timeout */
            wdt->enable();  /* 使能 */
        }
    }

    wdt->deinit();
    return 0;
}
