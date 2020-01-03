/*************************************************************************
	> Filename: sensor.h
	>   Author: qiuwei.wang
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Tue 21 Feb 2017 03:02:27 PM CST
 ************************************************************************/

#ifndef _SENSOR_H
#define _SENSOR_H

/*
 * Macros
 */
#define VERSION(pid, ver)   ((pid<<8)|(ver&0xFF))
#define GC2155_ID           0x2155
#define BF3703_ID           0x3703
#define OV7725_ID           0x7721
#define OV2640_ID           0x2642

/*
 * Sensor的I2C地址
 */
#define GC2155_I2C_ADDR     0x3c
#define BF3703_I2C_ADDR     0x6e
#define OV7725_I2C_ADDR     0x21
#define OV2640_I2c_ADDR     0x30

enum sensor_list {
    GC2155,
    BF3703,
    OV7725,
    OV2640,
};

struct sensor_info {
    enum sensor_list sensor;
    char *name;
};

enum image_width {
    W_QCIF  = 176,
    W_QVGA  = 320,
    W_CIF   = 352,
    W_VGA   = 640,
    W_SVGA  = 800,
    W_XGA   = 1024,
    W_720P  = 1280,
    W_SXGA  = 1280,
    W_UXGA  = 1600,
};

enum image_height {
    H_QCIF  = 144,
    H_QVGA  = 240,
    H_CIF   = 288,
    H_VGA   = 480,
    H_SVGA  = 600,
    H_720P  = 720,
    H_XGA   = 768,
    H_SXGA  = 1024,
    H_UXGA  = 1200,
};

struct image_fmt {
    char *name;
    enum image_width width;
    enum image_height height;
    const struct camera_regval_list *regs_val;
};

#define IMAGE_FMT(n, w, h, r) \
    {.name = n, .width = w , .height = h, .regs_val = r }

const struct image_fmt *select_image_fmt(uint32_t *width, uint32_t *height, \
                                         const struct image_fmt supported_image_fmts[]);
void sensor_config(enum sensor_list sensor , struct camera_manager *cm,  \
                   struct camera_img_param *img);

#endif /* _SENSOR_H */
