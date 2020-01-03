/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Thu 25 May 2017 08:32:59 PM CST
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
#include <74hc595/74hc595_manager.h>

#define LOG_TAG     "test_74hc595"
#define WRDATA_LEN  6


int main(int argc, char *argv[])
{
    uint16_t rddata;
    uint16_t wrdata[WRDATA_LEN] = {0xa55a, 0xbccb, 0x6688, 0x8899, 0x1ff1, 0x1234};
    uint32_t i = 0, out_bits;
    struct sn74hc595_manager *sn74hc595;

    sn74hc595 = get_sn74hc595_manager();
    sn74hc595->init(SN74HC595_DEV0);
    sn74hc595->get_outbits(SN74HC595_DEV0, &out_bits);

    sn74hc595->write(SN74HC595_DEV0, &wrdata[0], out_bits);
    sn74hc595->read(SN74HC595_DEV0, (void *)&rddata, out_bits);
    printf("rddata=0x%x\n", rddata);

    sn74hc595->clear(SN74HC595_DEV0);
    usleep(100*1000);

    while(1) {
        printf("wrdata=0x%x\n", wrdata[i]);
        sn74hc595->write(SN74HC595_DEV0, wrdata+i, out_bits);
        i++;
        i %= WRDATA_LEN;
        usleep(100*1000);
    }

    sn74hc595->deinit(SN74HC595_DEV0);
    return 0;
}
