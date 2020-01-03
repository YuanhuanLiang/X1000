/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Mon 19 Dec 2016 05:37:56 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <power/power_manager.h>


#define LOG_TAG   "test_power"

int main(int argc, char *argv[])
{
    char *opt = NULL;
    struct power_manager *pm;

    pm = get_power_manager();

    if(argc > 1)
        opt = argv[1];

    if(strcmp(opt, "poweroff") == 0)
        pm->power_off(); /* 关机 */

    else if(strcmp(opt, "reboot") == 0)
        pm->reboot();    /* 系统复位 */

    else if(strcmp(opt, "sleep") == 0)
        pm->sleep();     /* 进入休眠 */

    return 0;
}
