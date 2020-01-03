/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Mon 23 Dec 2016 03:37:56 PM CST
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
#include <efuse/efuse_manager.h>


#define LOG_TAG   "test_efuse"

enum operation {
    READ_EFUSE,
    WRITE_EFUSE,
};

void print_usage(void)
{
    printf("Usage: test_efuse [OPTION]\n\n"
           "-r id       --- read segment:\n"
           "                0-CHIP_ID\n"
           "                1-RANDOM_ID\n"
           "                2-USER_ID\n"
           "                3-PROTECT_ID\n"
           "-w id value --- write segment:\n"
           "                0-CHIP_ID\n"
           "                2-USER_ID\n"
           "                3-PROTECT_ID\n"
           "-help           get help message\n"
           "eg:\n"
           "    test_efuse -r 0      --- read efuse CHIP_ID\n"
           "    test_efuse -w -2 0x01 -- write efuse USER_ID\n"
           "\r\n");
}

double _ceil(double x)
{
    double y = x;

    if ((*(((int *) &y) + 1) & 0x80000000) != 0) {
        return (float) ((int) x);
    } else {
        if ((x - (int) x) < 0.000001) {
            return (float) ((int) x);
        } else {
            return (float) ((int) x) + 1;
        }
    }
}

int main(int argc, char *argv[])
{
    uint32_t i = 0;
    uint32_t length = 0;
    uint32_t *buf = NULL;
    enum operation oprt = -1;
    enum efuse_segment seg_id = -1;
    struct efuse_manager *efuse;

    if (argc <= 2) {
        print_usage();
        return -1;
    }

    if (argc > 0) {
        if (!strcmp("-r", argv[1]))
            oprt = READ_EFUSE;
        else if (!strcmp("-w", argv[1]))
            oprt = WRITE_EFUSE;
        else {
            print_usage();
            return -1;
        }

        seg_id = atoi(argv[2]);

        switch(seg_id) {
        case CHIP_ID:
            length = CHIP_ID_LENGTH;
            break;

        case RANDOM_ID:
            if (oprt == WRITE_EFUSE) {
                LOGE("Cannot to write random segment!\n");
                return -1;
            }
            length = RANDOM_ID_LENGTH;
            break;

        case USER_ID:
            length = USER_ID_LENGTH;
            break;

        case PROTECT_ID:
            length = PROTECT_ID_LENGTH;
            break;

        default:
            LOGE("Unknown segment ID: %d\n", seg_id);
            return -1;
        }
    }

    buf = (uint32_t *)malloc(length);
    if (!buf) {
        LOGE("Failed to malloc: %s\n", strerror(errno));
        return -1;
    }
    bzero(buf, length);

    /* 获取操作EFUSE的句柄 */
    efuse = get_efuse_manager();

    if (oprt == READ_EFUSE) {
        /* 读EFUSE指定的段 */
        efuse->read(seg_id, buf, length);
        for (i = 0; i < length / 4; i++)
            LOGI("Read buf[%02d] = 0x%08x\n", i, buf[i]);
    } else {
        char *val = argv[3];
        char temp[9] = "";
        double val_length;

        val_length = strlen(val);
        if (val_length > length)
            val_length = length * 2;

        for (i = 0; i < _ceil((double)(val_length / 8)); i++) {
            bzero(temp, sizeof(temp));
            strncpy(temp, val, (strlen(val) / 8) ? 8 : strlen(val) % 8);
            temp[8] = '\0';
            buf[i] = strtoul(temp, NULL, 16);
            LOGI("To write buf[%02d] = 0x%08x\n", i, buf[i]);
            val += 8;
        }

        /*
         * 写EFUSE
         * 写EFUSE需要硬件支持，并且所写段没有写保护，否则失败
         * 详细了解EFUSE，请查看芯片手册
         */
        efuse->write(seg_id, buf, length);
    }

    return 0;
}
