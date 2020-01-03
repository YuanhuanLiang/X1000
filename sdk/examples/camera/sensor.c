/*************************************************************************
	> Filename: sensor.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Tue 07 Mar 2017 10:33:31 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <camera/camera_manager.h>

#include "sensor.h"
#include "gc2155.h"
#include "bf3703.h"
#include "ov7725.h"
#include "ov2640.h"

#define LOG_TAG   "sensor"

/*
 * This function used to select the nearest higher resolution,
 * note that in this function may change the value of width and height.
 */
const struct image_fmt *select_image_fmt(uint32_t *width, uint32_t *height,
                                         const struct image_fmt supported_image_fmts[])
{
    int index = 0;
    while(supported_image_fmts[index].name &&
          supported_image_fmts[index].regs_val) {
        if (supported_image_fmts[index].width  >= *width &&
            supported_image_fmts[index].height >= *height) {
            *width = supported_image_fmts[index].width;
            *height = supported_image_fmts[index].height;
            return &supported_image_fmts[index];
        }

        index++;
    }

    *width = supported_image_fmts[0].width;
    *height = supported_image_fmts[0].height;
    return &supported_image_fmts[0];
}

void sensor_config(enum sensor_list sensor, struct camera_manager *cm, struct camera_img_param *img)
{
    uint8_t pid, vid;
    const struct image_fmt *fmt;

    switch(sensor) {
    /* Only for GC2155 */
    case GC2155:
        fmt = select_image_fmt(&img->width, &img->height, gc2155_supported_fmts);

        /* Set sensor device addr */
        cm->sensor_setup_addr(GC2155_I2C_ADDR);

        /* Probe sensor ID */
        pid = cm->sensor_read_reg(0xf0);
        vid = cm->sensor_read_reg(0xf1);
        if (VERSION(pid, vid) != GC2155_ID)
            goto probe_error;
        LOGE("GC2155 probe successed: pid = 0x%02x, vid = 0x%02x\n", pid, vid);

        /* 配置 sensor 的寄存器 */
        cm->sensor_setup_regs(gc2155_init_regs);
        cm->sensor_setup_regs(fmt->regs_val);
        break;

    /* Only for BF3703 */
    case BF3703:
        fmt = select_image_fmt(&img->width, &img->height, bf3703_supported_fmts);

        /* Set sensor device addr */
        cm->sensor_setup_addr(BF3703_I2C_ADDR);

        /* Probe sensor ID */
        pid = cm->sensor_read_reg(0xfc);
        vid = cm->sensor_read_reg(0xfd);
        if (VERSION(pid, vid) != BF3703_ID)
            goto probe_error;
        LOGE("BF3703 probe successed: pid = 0x%02x, vid = 0x%02x\n", pid, vid);

        /* Reset all registers */
        cm->sensor_write_reg(0x12, 0x80);
        usleep(2000);

        /* 配置 sensor 的寄存器 */
        cm->sensor_setup_regs(bf3703_init_regs);
        cm->sensor_setup_regs(fmt->regs_val);
        break;

    /* Only for OV7725 */
    case OV7725:
        fmt = select_image_fmt(&img->width, &img->height, ov7725_supported_fmts);

        /* Set sensor device addr */
        cm->sensor_setup_addr(OV7725_I2C_ADDR);

        /* Probe sensor ID */
        pid = cm->sensor_read_reg(0x0a);
        vid = cm->sensor_read_reg(0x0b);
        if (VERSION(pid, vid) != OV7725_ID)
            goto probe_error;
        LOGE("OV7725 probe successed: pid = 0x%02x, vid = 0x%02x\n", pid, vid);

        /* Reset all registers */
        cm->sensor_write_reg(0x12, 0x80);
        usleep(2000);

        /* 配置 sensor 的寄存器 */
        cm->sensor_setup_regs(ov7725_init_regs);
        cm->sensor_setup_regs(fmt->regs_val);
        break;

    /* Only for OV2640 */
    case OV2640:
        fmt = select_image_fmt(&img->width, &img->height, ov2640_supported_fmts);

        /* Set sensor device addr */
        cm->sensor_setup_addr(OV2640_I2c_ADDR);

        /* Probe sensor ID */
        pid = cm->sensor_read_reg(0x0a);
        vid = cm->sensor_read_reg(0x0b);
        if (VERSION(pid, vid) != OV2640_ID)
            goto probe_error;
        LOGE("OV2640 probe successed: pid = 0x%02x, vid = 0x%02x\n", pid, vid);

        /* Reset all registers */
        cm->sensor_write_reg(0xff, 0x01);
        cm->sensor_write_reg(0x12, 0x80);
        usleep(2000);

        /* Initiates sensor with default data */
        cm->sensor_setup_regs(ov2640_init_regs);

        /* Select preamble */
        cm->sensor_setup_regs(ov2640_size_change_preamble_regs);

        /* Set size win */
        cm->sensor_setup_regs(fmt->regs_val);

        /* cfmt preamble */
        cm->sensor_setup_regs(ov2640_format_change_preamble_regs);

        /* Set cfmt */
        cm->sensor_setup_regs(ov2640_yuyv_regs);
        break;

    /*Don't match any sensor */
    default:
        fprintf(stderr, "No support sensor: %d\n", sensor);
        exit(EXIT_FAILURE);
    }

    img->size = img->width * img->height * img->bpp / 8;
    putchar('\0'); // do nothing
    return;

probe_error:
    LOGE("Sensor probe failed: pid = 0x%02x, vid = 0x%02x\n", pid, vid);
    cm->camera_deinit();
    exit(EXIT_FAILURE);
}
