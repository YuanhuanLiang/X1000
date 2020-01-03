/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Tue 20 Dec 2016 02:07:10 PM CST
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

#include <utils/log.h>
#include <utils/assert.h>
#include <i2c/i2c_manager.h>

/*
 * Macros
 */
#define LOG_TAG           "test-i2c"
#define EEPROM_CHIP_ADDR  0x57


/*
 * Functions
 */
static void print_buf(unsigned char *buf, int size)
{
    int i;

    if(!buf)
        return;

    for(i = 0; i < size; i++)
        LOGI("buf[%d] = 0x%02x\n", i, buf[i]);

}

int main(int argc, char *argv[])
{
    unsigned char buf[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    struct i2c_unit i2c;
    struct i2c_manager *i2c_m;

    /* 获取操作I2C设备的句柄 */
    i2c_m = get_i2c_manager();

    /* 初始化I2C设备结构 */
    i2c.id = I2C1;
    i2c.chip_addr = EEPROM_CHIP_ADDR;

    i2c_m->init(&i2c);
    i2c_m->write(&i2c, buf, 0x100, sizeof(buf));

    /* 清零 */
    bzero(buf, sizeof(buf));
    print_buf(buf, sizeof(buf));

    LOGI("Read I2C-%d chip addr: 0x%02x\n", i2c.id, i2c.chip_addr);
    i2c_m->read(&i2c, buf, 0x100, sizeof(buf));
    print_buf(buf, sizeof(buf));

    i2c_m->deinit(&i2c);
    return 0;
}
