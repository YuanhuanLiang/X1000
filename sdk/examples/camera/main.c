/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Tue 13 Dec 2016 08:41:46 PM CST
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
#include <utils/image_convert.h>
#include <utils/assert.h>
#include <camera/camera_manager.h>

#include "sensor.h"

/*
 * Macros
 */
#define PRINT_IMAGE_DATA    0

#define DEFAULT_WIDTH    640
#define DEFAULT_HEIGHT   480
#define DEFAULT_BPP      16


#define LOG_TAG   "test_camera"


static struct sensor_info sensor_info[] = {
    {GC2155, "gc2155"},
    {BF3703, "bf3703"},
    {OV7725, "ov7725"},
    {OV2640, "ov2640"},
};

static const char short_options[] = "hx:y:s:";
static const struct option long_options[] = {
	{ "help",       0,      NULL,           'h' },
	{ "width",      1,      NULL,           'x' },
	{ "height",     1,      NULL,           'y' },
	{ "sensor",     1,      NULL,           's' },
	{ 0, 0, 0, 0 }
};

/*
 * Functions
 */
static void usage(FILE *fp, int argc, char *argv[])
{
    int i;
	fprintf(fp,
			 "\nUsage: %s [options]\n"
			 "Options:\n"
			 "-h | --help          Get the help messages\n"
			 "-x | --width         Set image width\n"
			 "-y | --height        Set image height\n"
             "-s | --sensor        Select sensor\n"
			 "\n", argv[0]);

    fprintf(fp, "Support sensor:\n");
    for(i = 0; i < sizeof(sensor_info)/sizeof(sensor_info[0]); i++) {
        fprintf(fp,
             "%d: %s\n", sensor_info[i].sensor, sensor_info[i].name);
    }
    putchar('\n');
}

#if (PRINT_IMAGE_DATA == 1)
static void print_data(unsigned char *pbuf, unsigned int image_size)
{
    int i;

    printf("unsigned char srcBuf[] = {");

    for(i = 0; i < image_size; i++) {
        if(i%46 == 0)
            printf("\n");
        printf("0x%02x,", *(pbuf + i));
    }

    printf("\n};\n");
    printf("\n----------- printf finish ----------\n");
}
#endif

int main(int argc, char *argv[])
{
    int ret;
    int cnt = 0;
    char filename[64] = "";
    uint8_t *yuvbuf = NULL;
    uint8_t *rgbbuf = NULL;
    enum sensor_list sensor = BF3703;
    struct camera_img_param img;
    struct camera_timing_param timing;
    struct camera_manager *cm;

    /* 获取操作摄像头句柄 */
    cm = get_camera_manager();
    cm->camera_init();

    /* 默认的分辨率和像素深度 */
    img.width  = DEFAULT_WIDTH;
    img.height = DEFAULT_HEIGHT;
    img.bpp    = DEFAULT_BPP;

    while(1) {
        int oc;
        oc = getopt_long(argc, argv, \
                         short_options, long_options, \
                         NULL);
        if (-1 == oc)
            break;

        switch(oc) {
        case 0:
            break;
        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);
            break;
        case 'x':
            img.width = atoi(optarg);
            break;
        case 'y':
            img.height = atoi(optarg);
            break;
        case 's':
            sensor = atoi(optarg);
            break;
        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    /* sensor probe and 寄存器配置 */
    sensor_config(sensor, cm, &img);

    /* 设置分辨率和像素深度 */
    cm->set_img_param(&img);

    /* 设置控制器时序和mclk频率 */
    timing.mclk_freq = 24000000;
    timing.pclk_active_level  = 1;
    timing.hsync_active_level = 1;
    timing.vsync_active_level = 1;
    cm->set_timing_param(&timing);

    /* 内存申请 */
    yuvbuf = (uint8_t *)malloc(img.size);
    if (!yuvbuf) {
        LOGE("Malloc yuvbuf failed: %s\n", strerror(errno));
        exit(-1);
    }

    rgbbuf = (uint8_t *)malloc(img.width * img.height * 3);
    if (!rgbbuf) {
        LOGE("Malloc rgbbuf failed: %s\n", strerror(errno));
        exit(-1);
    }

    while(1) {
        /* 开始获取图像 */
        ret = cm->camera_read(yuvbuf, img.size);
        LOGE("Camera read return value:%d\n", ret);
        if (ret != img.size) {
            /**
             * camera read failed
             */
            continue;
        }

        /* 处理图像*/
#if (PRINT_IMAGE_DATA == 1)
        print_data(yuvbuf, img.size);
#endif
        sprintf(filename, "test_%d.bmp", cnt);
        LOGI("filename = %s, cnt = %d\n", filename, cnt);

        convert_yuv2rgb(yuvbuf, rgbbuf, img.width, img.height);
        convert_rgb2bmp(filename, img.width, img.height, 24, rgbbuf);

        cnt++;
        if (cnt > 5)
            break;
    }

    /* 释放内存和摄像头 */
    free(yuvbuf);
    free(rgbbuf);
    cm->camera_deinit();
    return 0;
}
