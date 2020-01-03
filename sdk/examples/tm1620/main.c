/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Wed 16 May 2018 12:03:47 PM CST
 ************************************************************************/

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

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>
#include <tm1620/tm1620_manager.h>

#define LOG_TAG    "test_tm1620"

uint8_t display_code[] = {
    0x3f, 0x06, 0x5b, 0x4f, /* 0 ~ 3 */
    0x66, 0x6d, 0x7d, 0x07, /* 4 ~ 7 */
    0x7f, 0x6f, 0x77, 0x7c, /* 8 ~ b */
    0x39, 0x5e, 0x79, 0x71  /* c ~ f */
};

int main(void)
{
    uint8_t display_data[6] = {0};
    struct tm1620_manager *tm1620;

    tm1620 = get_tm1620_manager();
    if (tm1620->init() < 0) {
        LOGE("Failed to init TM1620\n");
        return -1;
    }

    /* set digital display '1' ~ '6' */
    display_data[0] = display_code[1];
    display_data[1] = display_code[2];
    display_data[2] = display_code[3];
    display_data[3] = display_code[4];
    display_data[4] = display_code[5];
    display_data[5] = display_code[6];
    tm1620->all_display(display_data);
    sleep(2);

    /* set sixth grid display 'f' */
    tm1620->grid_display(6, display_code[15]);
    sleep(2);

    /* close all display */
    tm1620->close_display();
    sleep(2);

    /* open all display */
    tm1620->open_display();
    sleep(2);

    /* set first grid display '9' */
    tm1620->grid_display(1, display_code[9]);
    sleep(2);

    tm1620->deinit();
	return 0;
}
