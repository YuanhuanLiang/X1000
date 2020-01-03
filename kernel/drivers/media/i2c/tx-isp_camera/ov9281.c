/*
 * ov9281 Camera Driver
 *
 * Copyright (C) 2018, Ingenic Semiconductor Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <soc/gpio.h>

#include <tx-isp/tx-isp-common.h>
#include <tx-isp/sensor-common.h>


#define OV9281_CHIP_ID_H               (0x92)
#define OV9281_CHIP_ID_L               (0x81)

#undef OV9281_720P_SUPPORT
#ifdef OV9281_720P_SUPPORT
#define OV9281_SCLK                     (40000000)
#define SENSOR_OUTPUT_MAX_FPS           60
#else
#define OV9281_SCLK                     (80000000)
#define SENSOR_OUTPUT_MAX_FPS           120
#endif
#define SENSOR_OUTPUT_MIN_FPS           5
#define SENSOR_VERSION                  "H20181220"


static int dvdd_en_gpio = GPIO_PB(29);
static int pwdn_gpio = GPIO_PA(19);


enum camera_reg_ops {
    CAMERA_REG_OP_DATA                  = 1,
    CAMERA_REG_OP_DELAY,
    CAMERA_REG_OP_END,
};

struct camera_reg_op {
    u32 flag;
    u16 reg;
    u16 val;
};


/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
    unsigned int value;
    unsigned int gain;
};
struct again_lut ov9281_again_lut[] = {
    {0x10, 0},
    {0x11, 5731},
    {0x12, 11136},
    {0x13, 16247},
    {0x14, 21097},
    {0x15, 25710},
    {0x16, 30108},
    {0x17, 34311},
    {0x18, 38335},
    {0x19, 42195},
    {0x1a, 45903},
    {0x1b, 49471},
    {0x1c, 52910},
    {0x1d, 56227},
    {0x1e, 59433},
    {0x1f, 62533},
    {0x20, 65535},
    {0x21, 68444},
    {0x22, 71266},
    {0x23, 74007},
    {0x24, 76671},
    {0x25, 79261},
    {0x26, 81782},
    {0x27, 84238},
    {0x28, 86632},
    {0x29, 88967},
    {0x2a, 91245},
    {0x2b, 93470},
    {0x2c, 95643},
    {0x2d, 97768},
    {0x2e, 99846},
    {0x2f, 101879},
    {0x30, 103870},
    {0x31, 105820},
    {0x32, 107730},
    {0x33, 109602},
    {0x34, 111438},
    {0x35, 113239},
    {0x36, 115006},
    {0x37, 116741},
    {0x38, 118445},
    {0x39, 120118},
    {0x3a, 121762},
    {0x3b, 123379},
    {0x3c, 124968},
    {0x3d, 126530},
    {0x3e, 128068},
    {0x3f, 129581},
    {0x40, 131070},
    {0x41, 132535},
    {0x42, 133979},
    {0x43, 135401},
    {0x44, 136801},
    {0x45, 138182},
    {0x46, 139542},
    {0x47, 140883},
    {0x48, 142206},
    {0x49, 143510},
    {0x4a, 144796},
    {0x4b, 146065},
    {0x4c, 147317},
    {0x4d, 148553},
    {0x4e, 149773},
    {0x4f, 150978},
    {0x50, 152167},
    {0x51, 153342},
    {0x52, 154502},
    {0x53, 155648},
    {0x54, 156780},
    {0x55, 157899},
    {0x56, 159005},
    {0x57, 160098},
    {0x58, 161178},
    {0x59, 162247},
    {0x5a, 163303},
    {0x5b, 164348},
    {0x5c, 165381},
    {0x5d, 166403},
    {0x5e, 167414},
    {0x5f, 168415},
    {0x60, 169405},
    {0x61, 170385},
    {0x62, 171355},
    {0x63, 172314},
    {0x64, 173265},
    {0x65, 174205},
    {0x66, 175137},
    {0x67, 176059},
    {0x68, 176973},
    {0x69, 177878},
    {0x6a, 178774},
    {0x6b, 179662},
    {0x6c, 180541},
    {0x6d, 181412},
    {0x6e, 182276},
    {0x6f, 183132},
    {0x70, 183980},
    {0x71, 184820},
    {0x72, 185653},
    {0x73, 186479},
    {0x74, 187297},
    {0x75, 188109},
    {0x76, 188914},
    {0x77, 189711},
    {0x78, 190503},
    {0x79, 191287},
    {0x7a, 192065},
    {0x7b, 192837},
    {0x7c, 193603},
    {0x7d, 194362},
    {0x7e, 195116},
    {0x7f, 195863},
    {0x80, 196605},
    {0x81, 197340},
    {0x82, 198070},
    {0x83, 198795},
    {0x84, 199514},
    {0x85, 200227},
    {0x86, 200936},
    {0x87, 201639},
    {0x88, 202336},
    {0x89, 203029},
    {0x8a, 203717},
    {0x8b, 204399},
    {0x8c, 205077},
    {0x8d, 205750},
    {0x8e, 206418},
    {0x8f, 207082},
    {0x90, 207741},
    {0x91, 208395},
    {0x92, 209045},
    {0x93, 209690},
    {0x94, 210331},
    {0x95, 210968},
    {0x96, 211600},
    {0x97, 212228},
    {0x98, 212852},
    {0x99, 213472},
    {0x9a, 214088},
    {0x9b, 214700},
    {0x9c, 215308},
    {0x9d, 215912},
    {0x9e, 216513},
    {0x9f, 217109},
    {0xa0, 217702},
    {0xa1, 218291},
    {0xa2, 218877},
    {0xa3, 219458},
    {0xa4, 220037},
    {0xa5, 220611},
    {0xa6, 221183},
    {0xa7, 221751},
    {0xa8, 222315},
    {0xa9, 222876},
    {0xaa, 223434},
    {0xab, 223988},
    {0xac, 224540},
    {0xad, 225088},
    {0xae, 225633},
    {0xaf, 226175},
    {0xb0, 226713},
    {0xb1, 227249},
    {0xb2, 227782},
    {0xb3, 228311},
    {0xb4, 228838},
    {0xb5, 229362},
    {0xb6, 229883},
    {0xb7, 230401},
    {0xb8, 230916},
    {0xb9, 231429},
    {0xba, 231938},
    {0xbb, 232445},
    {0xbc, 232949},
    {0xbd, 233451},
    {0xbe, 233950},
    {0xbf, 234446},
    {0xc0, 234940},
    {0xc1, 235431},
    {0xc2, 235920},
    {0xc3, 236406},
    {0xc4, 236890},
    {0xc5, 237371},
    {0xc6, 237849},
    {0xc7, 238326},
    {0xc8, 238800},
    {0xc9, 239271},
    {0xca, 239740},
    {0xcb, 240207},
    {0xcc, 240672},
    {0xcd, 241134},
    {0xce, 241594},
    {0xcf, 242052},
    {0xd0, 242508},
    {0xd1, 242961},
    {0xd2, 243413},
    {0xd3, 243862},
    {0xd4, 244309},
    {0xd5, 244754},
    {0xd6, 245197},
    {0xd7, 245637},
    {0xd8, 246076},
    {0xd9, 246513},
    {0xda, 246947},
    {0xdb, 247380},
    {0xdc, 247811},
    {0xdd, 248240},
    {0xde, 248667},
    {0xdf, 249091},
    {0xe0, 249515},
    {0xe1, 249936},
    {0xe2, 250355},
    {0xe3, 250772},
    {0xe4, 251188},
    {0xe5, 251602},
    {0xe6, 252014},
    {0xe7, 252424},
    {0xe8, 252832},
    {0xe9, 253239},
    {0xea, 253644},
    {0xeb, 254047},
    {0xec, 254449},
    {0xed, 254848},
    {0xee, 255246},
    {0xef, 255643},
    {0xf0, 256038},
    {0xf1, 256431},
    {0xf2, 256822},
    {0xf3, 257212},
    {0xf4, 257600},
    {0xf5, 257987},
    {0xf6, 258372},
    {0xf7, 258756},
    {0xf8, 259138},
};

struct tx_isp_sensor_attribute ov9281_attr;
unsigned int ov9281_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
    struct again_lut *lut = ov9281_again_lut;

    while(lut->gain <= ov9281_attr.max_again) {
        if(isp_gain == 0) {
            *sensor_again = lut->value;
            return 0;
        }
        else if(isp_gain < lut->gain) {
            *sensor_again = (lut - 1)->value;
            return (lut - 1)->gain;
        }
        else{
            if((lut->gain == ov9281_attr.max_again) && (isp_gain >= lut->gain)) {
                *sensor_again = lut->value;
                return lut->gain;
            }
        }

        lut++;
    }

    return isp_gain;
}

unsigned int ov9281_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
    return isp_gain;
}


struct tx_isp_sensor_attribute ov9281_attr = {
    .name = "ov9281",
    .chip_id = 0x9281,
    .cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
    .cbus_mask = V4L2_SBUS_MASK_SAMPLE_8BITS | V4L2_SBUS_MASK_ADDR_16BITS,
    .cbus_device = 0x60,
    .dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
    .mipi = {
        .clk = 800,
        .lans = 1,
    },
    .max_again = 259138,
    .max_dgain = 0,
    .min_integration_time = 2,
    .min_integration_time_native = 2,
    .max_integration_time_native = 915 - 4,
    .integration_time_limit = 915 - 4,
    .total_width = 728,
    .total_height = 915,
    .max_integration_time = 915 - 4,
    .integration_time_apply_delay = 2,
    .again_apply_delay = 2,
    .dgain_apply_delay = 0,
    .sensor_ctrl.alloc_again = ov9281_alloc_again,
    .sensor_ctrl.alloc_dgain = ov9281_alloc_dgain,
    //void priv; /* point to struct tx_isp_sensor_board_info */
};


/*
 * MCLK=24Mhz, MIPI_clcok=800MHz
 * Actual_window_size=640*480,
 * SCLK=80MHz, Pixel_line=728, row_time=9.1us
 * line_frame=915, frame_rate=120fps
 */
static struct camera_reg_op ov9281_init_regs_640_480_120fps_mipi[] = {
    {CAMERA_REG_OP_DATA,0x0103,0x01},
    {CAMERA_REG_OP_DATA,0x0302,0x32},
    {CAMERA_REG_OP_DATA,0x030d,0x50},
    {CAMERA_REG_OP_DATA,0x030e,0x02},
    {CAMERA_REG_OP_DATA,0x3001,0x00},
    {CAMERA_REG_OP_DATA,0x3004,0x00},
    {CAMERA_REG_OP_DATA,0x3005,0x00},
    {CAMERA_REG_OP_DATA,0x3006,0x04},
    {CAMERA_REG_OP_DATA,0x3011,0x0a},
    {CAMERA_REG_OP_DATA,0x3013,0x18},
    {CAMERA_REG_OP_DATA,0x301c,0xf0},
    {CAMERA_REG_OP_DATA,0x3022,0x01},
    {CAMERA_REG_OP_DATA,0x3030,0x10},
    {CAMERA_REG_OP_DATA,0x3039,0x12}, //mipi, 1-lane mode
    {CAMERA_REG_OP_DATA,0x303a,0x00},
    {CAMERA_REG_OP_DATA,0x3500,0x00}, //exposure[19:16]
    {CAMERA_REG_OP_DATA,0x3501,0x2a}, //exposure[15:8]
    {CAMERA_REG_OP_DATA,0x3502,0x90}, //exposure[7:0], low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3503,0x08},
    {CAMERA_REG_OP_DATA,0x3505,0x8c},
    {CAMERA_REG_OP_DATA,0x3507,0x03}, //gain shift, << 3
    {CAMERA_REG_OP_DATA,0x3508,0x00},
    {CAMERA_REG_OP_DATA,0x3509,0x10}, //gain,  low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3610,0x80},
    {CAMERA_REG_OP_DATA,0x3611,0xa0},
    {CAMERA_REG_OP_DATA,0x3620,0x6e},
    {CAMERA_REG_OP_DATA,0x3632,0x56},
    {CAMERA_REG_OP_DATA,0x3633,0x78},
    {CAMERA_REG_OP_DATA,0x3662,0x01}, //raw10, 1-lane select
    {CAMERA_REG_OP_DATA,0x3666,0x00},
    {CAMERA_REG_OP_DATA,0x366f,0x5a},
    {CAMERA_REG_OP_DATA,0x3680,0x84},
    {CAMERA_REG_OP_DATA,0x3712,0x80},
    {CAMERA_REG_OP_DATA,0x372d,0x22},
    {CAMERA_REG_OP_DATA,0x3731,0x80},
    {CAMERA_REG_OP_DATA,0x3732,0x30},
    {CAMERA_REG_OP_DATA,0x3778,0x00},
    {CAMERA_REG_OP_DATA,0x377d,0x22},
    {CAMERA_REG_OP_DATA,0x3788,0x02},
    {CAMERA_REG_OP_DATA,0x3789,0xa4},
    {CAMERA_REG_OP_DATA,0x378a,0x00},
    {CAMERA_REG_OP_DATA,0x378b,0x4a},
    {CAMERA_REG_OP_DATA,0x3799,0x20},

    {CAMERA_REG_OP_DATA,0x3800,0x00}, //X_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3801,0x00}, //X_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3802,0x00}, //Y_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3803,0x00}, //Y_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3804,0x05}, //X_ADDR_END_H : 1295
    {CAMERA_REG_OP_DATA,0x3805,0x0f}, //X_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3806,0x03}, //Y_ADDR_END_H : 815
    {CAMERA_REG_OP_DATA,0x3807,0x2f}, //Y_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3808,0x02}, //X_OUTPUT_SIZE_H, isp : 640
    {CAMERA_REG_OP_DATA,0x3809,0x80}, //X_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380a,0x01}, //Y_OUTPUT_SIZE_H, isp : 480
    {CAMERA_REG_OP_DATA,0x380b,0xe0}, //Y_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380c,0x02}, //HTS_H : 728
    {CAMERA_REG_OP_DATA,0x380d,0xd8}, //HTS_L
    {CAMERA_REG_OP_DATA,0x380e,0x03}, //VTS_H : 915
    {CAMERA_REG_OP_DATA,0x380f,0x93}, //VTS_L
    {CAMERA_REG_OP_DATA,0x3810,0x00}, //ISP_X_WIN_H : 8
    {CAMERA_REG_OP_DATA,0x3811,0x08}, //ISP_X_WIN_L
    {CAMERA_REG_OP_DATA,0x3812,0x00}, //ISP_Y_WIN_H : 8
    {CAMERA_REG_OP_DATA,0x3813,0x08}, //ISP_Y_WIN_L
    {CAMERA_REG_OP_DATA,0x3814,0x11},
    {CAMERA_REG_OP_DATA,0x3815,0x11},
    {CAMERA_REG_OP_DATA,0x3820,0x40},
    {CAMERA_REG_OP_DATA,0x3821,0x00},
    {CAMERA_REG_OP_DATA,0x3881,0x42},
    {CAMERA_REG_OP_DATA,0x3882,0x01},
    {CAMERA_REG_OP_DATA,0x3883,0x00},
    {CAMERA_REG_OP_DATA,0x3885,0x02},
    {CAMERA_REG_OP_DATA,0x38a8,0x02},
    {CAMERA_REG_OP_DATA,0x38a9,0x80},
    {CAMERA_REG_OP_DATA,0x38b1,0x00},
    {CAMERA_REG_OP_DATA,0x38c4,0x00},
    {CAMERA_REG_OP_DATA,0x38c5,0xc0},
    {CAMERA_REG_OP_DATA,0x38c6,0x04},
    {CAMERA_REG_OP_DATA,0x38c7,0x80},
    {CAMERA_REG_OP_DATA,0x3920,0xff},
    {CAMERA_REG_OP_DATA,0x4003,0x40},
    {CAMERA_REG_OP_DATA,0x4008,0x04},
    {CAMERA_REG_OP_DATA,0x4009,0x0b},
    {CAMERA_REG_OP_DATA,0x400c,0x00},
    {CAMERA_REG_OP_DATA,0x400d,0x07},
    {CAMERA_REG_OP_DATA,0x4010,0x40},
    {CAMERA_REG_OP_DATA,0x4043,0x40},
    {CAMERA_REG_OP_DATA,0x4307,0x30},
    {CAMERA_REG_OP_DATA,0x4317,0x00},
    {CAMERA_REG_OP_DATA,0x4501,0x00},
    {CAMERA_REG_OP_DATA,0x4507,0x00},
    {CAMERA_REG_OP_DATA,0x4509,0x00},
    {CAMERA_REG_OP_DATA,0x450a,0x08},
    {CAMERA_REG_OP_DATA,0x4601,0x04},
    {CAMERA_REG_OP_DATA,0x470f,0x00},
    {CAMERA_REG_OP_DATA,0x4f07,0x00},
    {CAMERA_REG_OP_DATA,0x4800,0x00},
    {CAMERA_REG_OP_DATA,0x4837,0x15},
    {CAMERA_REG_OP_DATA,0x5000,0x9f},
    {CAMERA_REG_OP_DATA,0x5001,0x00},
    {CAMERA_REG_OP_DATA,0x5e00,0x00},
    {CAMERA_REG_OP_DATA,0x5d00,0x07},
    {CAMERA_REG_OP_DATA,0x5d01,0x00},
    //{CAMERA_REG_OP_DATA,0x0100,0x01}, //streaming
    {CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};

/*
 * MCLK=24Mhz, MIPI_clcok=800MHz
 * Actual_window_size=1280*720,
 * SCLK=40MHz, Pixel_line=728, row_time18.2us
 * line_frame=915, frame_rate=60fps
 */
static struct camera_reg_op ov9281_init_regs_1280_720_60fps_mipi[] = {
    {CAMERA_REG_OP_DATA,0x0103,0x01},
    {CAMERA_REG_OP_DATA,0x0302,0x32},
    {CAMERA_REG_OP_DATA,0x030d,0x50},
    {CAMERA_REG_OP_DATA,0x030e,0x06},
    {CAMERA_REG_OP_DATA,0x3001,0x00},
    {CAMERA_REG_OP_DATA,0x3004,0x00},
    {CAMERA_REG_OP_DATA,0x3005,0x00},
    {CAMERA_REG_OP_DATA,0x3006,0x04},
    {CAMERA_REG_OP_DATA,0x3011,0x0a},
    {CAMERA_REG_OP_DATA,0x3013,0x18},
    {CAMERA_REG_OP_DATA,0x301c,0xf0},
    {CAMERA_REG_OP_DATA,0x3022,0x01},
    {CAMERA_REG_OP_DATA,0x3030,0x10},
    {CAMERA_REG_OP_DATA,0x3039,0x12}, //mipi, 1-lane mode
    {CAMERA_REG_OP_DATA,0x303a,0x00},
    {CAMERA_REG_OP_DATA,0x3500,0x00}, //exposure[19:16]
    {CAMERA_REG_OP_DATA,0x3501,0x2a}, //exposure[15:8]
    {CAMERA_REG_OP_DATA,0x3502,0x90}, //exposure[7:0], low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3503,0x08},
    {CAMERA_REG_OP_DATA,0x3505,0x8c},
    {CAMERA_REG_OP_DATA,0x3507,0x03}, //gain shift, << 3
    {CAMERA_REG_OP_DATA,0x3508,0x00},
    {CAMERA_REG_OP_DATA,0x3509,0x10}, //gain,  low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3610,0x80},
    {CAMERA_REG_OP_DATA,0x3611,0xa0},
    {CAMERA_REG_OP_DATA,0x3620,0x6e},
    {CAMERA_REG_OP_DATA,0x3632,0x56},
    {CAMERA_REG_OP_DATA,0x3633,0x78},
    {CAMERA_REG_OP_DATA,0x3662,0x01}, //raw10, 1-lane select
    {CAMERA_REG_OP_DATA,0x3666,0x00},
    {CAMERA_REG_OP_DATA,0x366f,0x5a},
    {CAMERA_REG_OP_DATA,0x3680,0x84},
    {CAMERA_REG_OP_DATA,0x3712,0x80},
    {CAMERA_REG_OP_DATA,0x372d,0x22},
    {CAMERA_REG_OP_DATA,0x3731,0x80},
    {CAMERA_REG_OP_DATA,0x3732,0x30},
    {CAMERA_REG_OP_DATA,0x3778,0x00},
    {CAMERA_REG_OP_DATA,0x377d,0x22},
    {CAMERA_REG_OP_DATA,0x3788,0x02},
    {CAMERA_REG_OP_DATA,0x3789,0xa4},
    {CAMERA_REG_OP_DATA,0x378a,0x00},
    {CAMERA_REG_OP_DATA,0x378b,0x4a},
    {CAMERA_REG_OP_DATA,0x3799,0x20},

    {CAMERA_REG_OP_DATA,0x3800,0x00}, //X_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3801,0x00}, //X_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3802,0x00}, //Y_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3803,0x00}, //Y_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3804,0x05}, //X_ADDR_END_H : 1295
    {CAMERA_REG_OP_DATA,0x3805,0x0f}, //X_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3806,0x03}, //Y_ADDR_END_H : 815
    {CAMERA_REG_OP_DATA,0x3807,0x2f}, //Y_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3808,0x05}, //X_OUTPUT_SIZE_H, isp : 1280
    {CAMERA_REG_OP_DATA,0x3809,0x00}, //X_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380a,0x02}, //Y_OUTPUT_SIZE_H, isp : 720
    {CAMERA_REG_OP_DATA,0x380b,0xd0}, //Y_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380c,0x02}, //HTS_H : 728
    {CAMERA_REG_OP_DATA,0x380d,0xd8}, //HTS_L
    {CAMERA_REG_OP_DATA,0x380e,0x03}, //VTS_H : 915
    {CAMERA_REG_OP_DATA,0x380f,0x93}, //VTS_L
    {CAMERA_REG_OP_DATA,0x3810,0x00}, //ISP_X_WIN_H : 8
    {CAMERA_REG_OP_DATA,0x3811,0x08}, //ISP_X_WIN_L
    {CAMERA_REG_OP_DATA,0x3812,0x00}, //ISP_Y_WIN_H : 8
    {CAMERA_REG_OP_DATA,0x3813,0x08}, //ISP_Y_WIN_L
    {CAMERA_REG_OP_DATA,0x3814,0x11},
    {CAMERA_REG_OP_DATA,0x3815,0x11},
    {CAMERA_REG_OP_DATA,0x3820,0x40},
    {CAMERA_REG_OP_DATA,0x3821,0x00},
    {CAMERA_REG_OP_DATA,0x3881,0x42},
    {CAMERA_REG_OP_DATA,0x3882,0x01},
    {CAMERA_REG_OP_DATA,0x3883,0x00},
    {CAMERA_REG_OP_DATA,0x3885,0x02},
    {CAMERA_REG_OP_DATA,0x38a8,0x02},
    {CAMERA_REG_OP_DATA,0x38a9,0x80},
    {CAMERA_REG_OP_DATA,0x38b1,0x00},
    {CAMERA_REG_OP_DATA,0x38c4,0x00},
    {CAMERA_REG_OP_DATA,0x38c5,0xc0},
    {CAMERA_REG_OP_DATA,0x38c6,0x04},
    {CAMERA_REG_OP_DATA,0x38c7,0x80},
    {CAMERA_REG_OP_DATA,0x3920,0xff},
    {CAMERA_REG_OP_DATA,0x4003,0x40},
    {CAMERA_REG_OP_DATA,0x4008,0x04},
    {CAMERA_REG_OP_DATA,0x4009,0x0b},
    {CAMERA_REG_OP_DATA,0x400c,0x00},
    {CAMERA_REG_OP_DATA,0x400d,0x07},
    {CAMERA_REG_OP_DATA,0x4010,0x40},
    {CAMERA_REG_OP_DATA,0x4043,0x40},
    {CAMERA_REG_OP_DATA,0x4307,0x30},
    {CAMERA_REG_OP_DATA,0x4317,0x00},
    {CAMERA_REG_OP_DATA,0x4501,0x00},
    {CAMERA_REG_OP_DATA,0x4507,0x00},
    {CAMERA_REG_OP_DATA,0x4509,0x00},
    {CAMERA_REG_OP_DATA,0x450a,0x08},
    {CAMERA_REG_OP_DATA,0x4601,0x04},
    {CAMERA_REG_OP_DATA,0x470f,0x00},
    {CAMERA_REG_OP_DATA,0x4f07,0x00},
    {CAMERA_REG_OP_DATA,0x4800,0x00},
    {CAMERA_REG_OP_DATA,0x4837,0x15},
    {CAMERA_REG_OP_DATA,0x5000,0x9f},
    {CAMERA_REG_OP_DATA,0x5001,0x00},
    {CAMERA_REG_OP_DATA,0x5e00,0x00},
    {CAMERA_REG_OP_DATA,0x5d00,0x07},
    {CAMERA_REG_OP_DATA,0x5d01,0x00},
    //{CAMERA_REG_OP_DATA,0x0100,0x01}, //streaming
    {CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};

/*
 * MCLK=24Mhz, MIPI_clcok=800MHz
 * Actual_window_size=640*400,
 * SCLK=80MHz, Pixel_line=728, row_time=9.1us
 * line_frame=915, frame_rate=120fps
 *
 * note: this setting max resolution is 640*400
 */
static struct camera_reg_op ov9281_init_regs_640_400_120fps_mipi[] = {
    {CAMERA_REG_OP_DATA,0x0103,0x01},
    {CAMERA_REG_OP_DATA,0x0302,0x32},
    {CAMERA_REG_OP_DATA,0x030d,0x50},
    {CAMERA_REG_OP_DATA,0x030e,0x02},
    {CAMERA_REG_OP_DATA,0x3001,0x00},
    {CAMERA_REG_OP_DATA,0x3004,0x00},
    {CAMERA_REG_OP_DATA,0x3005,0x00},
    {CAMERA_REG_OP_DATA,0x3006,0x04},
    {CAMERA_REG_OP_DATA,0x3011,0x0a},
    {CAMERA_REG_OP_DATA,0x3013,0x18},
    {CAMERA_REG_OP_DATA,0x301c,0xf0},
    {CAMERA_REG_OP_DATA,0x3022,0x01},
    {CAMERA_REG_OP_DATA,0x3030,0x10},
    {CAMERA_REG_OP_DATA,0x3039,0x12}, //mipi, 1-lane mode
    {CAMERA_REG_OP_DATA,0x303a,0x00},
    {CAMERA_REG_OP_DATA,0x3500,0x00}, //exposure[19:16]
    {CAMERA_REG_OP_DATA,0x3501,0x01}, //exposure[15:8]
    {CAMERA_REG_OP_DATA,0x3502,0xf4}, //exposure[7:0], low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3503,0x08},
    {CAMERA_REG_OP_DATA,0x3505,0x8c},
    {CAMERA_REG_OP_DATA,0x3507,0x03}, //gain shift, << 3
    {CAMERA_REG_OP_DATA,0x3508,0x00},
    {CAMERA_REG_OP_DATA,0x3509,0x10}, //gain,  low 4 bits are fraction bits
    {CAMERA_REG_OP_DATA,0x3610,0x80},
    {CAMERA_REG_OP_DATA,0x3611,0xa0},
    {CAMERA_REG_OP_DATA,0x3620,0x6e},
    {CAMERA_REG_OP_DATA,0x3632,0x56},
    {CAMERA_REG_OP_DATA,0x3633,0x78},
    {CAMERA_REG_OP_DATA,0x3662,0x01}, //raw10, 1-lane select
    {CAMERA_REG_OP_DATA,0x3666,0x00},
    {CAMERA_REG_OP_DATA,0x366f,0x5a},
    {CAMERA_REG_OP_DATA,0x3680,0x84},
    {CAMERA_REG_OP_DATA,0x3712,0x80},
    {CAMERA_REG_OP_DATA,0x372d,0x22},
    {CAMERA_REG_OP_DATA,0x3731,0x80},
    {CAMERA_REG_OP_DATA,0x3732,0x30},
    {CAMERA_REG_OP_DATA,0x3778,0x10},
    {CAMERA_REG_OP_DATA,0x377d,0x22},
    {CAMERA_REG_OP_DATA,0x3788,0x02},
    {CAMERA_REG_OP_DATA,0x3789,0xa4},
    {CAMERA_REG_OP_DATA,0x378a,0x00},
    {CAMERA_REG_OP_DATA,0x378b,0x4a},
    {CAMERA_REG_OP_DATA,0x3799,0x20},

    {CAMERA_REG_OP_DATA,0x3800,0x00}, //X_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3801,0x00}, //X_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3802,0x00}, //Y_ADDR_START_H : 0
    {CAMERA_REG_OP_DATA,0x3803,0x00}, //Y_ADDR_START_L
    {CAMERA_REG_OP_DATA,0x3804,0x05}, //X_ADDR_END_H : 1295
    {CAMERA_REG_OP_DATA,0x3805,0x0f}, //X_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3806,0x03}, //Y_ADDR_END_H : 815
    {CAMERA_REG_OP_DATA,0x3807,0x2f}, //Y_ADDR_END_L
    {CAMERA_REG_OP_DATA,0x3808,0x02}, //X_OUTPUT_SIZE_H : 640
    {CAMERA_REG_OP_DATA,0x3809,0x80}, //X_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380a,0x01}, //Y_OUTPUT_SIZE_H : 400
    {CAMERA_REG_OP_DATA,0x380b,0x90}, //Y_OUTPUT_SIZE_L
    {CAMERA_REG_OP_DATA,0x380c,0x02}, //HTS_H : 728
    {CAMERA_REG_OP_DATA,0x380d,0xd8}, //HTS_L
    {CAMERA_REG_OP_DATA,0x380e,0x03}, //VTS_H : 915
    {CAMERA_REG_OP_DATA,0x380f,0x93}, //VTS_L
    {CAMERA_REG_OP_DATA,0x3810,0x00}, //ISP_X_WIN_H : 4
    {CAMERA_REG_OP_DATA,0x3811,0x04}, //ISP_X_WIN_L
    {CAMERA_REG_OP_DATA,0x3812,0x00}, //ISP_Y_WIN_H : 4
    {CAMERA_REG_OP_DATA,0x3813,0x04}, //ISP_Y_WIN_L

    {CAMERA_REG_OP_DATA,0x3814,0x31},
    {CAMERA_REG_OP_DATA,0x3815,0x22},
    {CAMERA_REG_OP_DATA,0x3820,0x60},
    {CAMERA_REG_OP_DATA,0x3821,0x01},
    {CAMERA_REG_OP_DATA,0x382c,0x05},
    {CAMERA_REG_OP_DATA,0x382d,0xb0},
    {CAMERA_REG_OP_DATA,0x389d,0x00},
    {CAMERA_REG_OP_DATA,0x3881,0x42},
    {CAMERA_REG_OP_DATA,0x3882,0x01},
    {CAMERA_REG_OP_DATA,0x3883,0x00},
    {CAMERA_REG_OP_DATA,0x3885,0x02},
    {CAMERA_REG_OP_DATA,0x38a8,0x02},
    {CAMERA_REG_OP_DATA,0x38a9,0x80},
    {CAMERA_REG_OP_DATA,0x38b1,0x00},
    {CAMERA_REG_OP_DATA,0x38b3,0x02},
    {CAMERA_REG_OP_DATA,0x38c4,0x00},
    {CAMERA_REG_OP_DATA,0x38c5,0xc0},
    {CAMERA_REG_OP_DATA,0x38c6,0x04},
    {CAMERA_REG_OP_DATA,0x38c7,0x80},
    {CAMERA_REG_OP_DATA,0x3920,0xff},
    {CAMERA_REG_OP_DATA,0x4003,0x40},
    {CAMERA_REG_OP_DATA,0x4008,0x02},
    {CAMERA_REG_OP_DATA,0x4009,0x05},
    {CAMERA_REG_OP_DATA,0x400c,0x00},
    {CAMERA_REG_OP_DATA,0x400d,0x03},
    {CAMERA_REG_OP_DATA,0x4010,0x40},
    {CAMERA_REG_OP_DATA,0x4043,0x40},
    {CAMERA_REG_OP_DATA,0x4307,0x30},
    {CAMERA_REG_OP_DATA,0x4317,0x00},
    {CAMERA_REG_OP_DATA,0x4501,0x00},
    {CAMERA_REG_OP_DATA,0x4507,0x03},
    {CAMERA_REG_OP_DATA,0x4509,0x80},
    {CAMERA_REG_OP_DATA,0x450a,0x08},
    {CAMERA_REG_OP_DATA,0x4601,0x04},
    {CAMERA_REG_OP_DATA,0x470f,0x00},
    {CAMERA_REG_OP_DATA,0x4f07,0x00},
    {CAMERA_REG_OP_DATA,0x4800,0x00},

#if 0
    {CAMERA_REG_OP_DATA,0x4802,0x84},
    //{CAMERA_REG_OP_DATA,0x4818,0x00}, //hs_zero_min
    {CAMERA_REG_OP_DATA,0x4819,0x0c},
    //{CAMERA_REG_OP_DATA,0x4824,0x01}, //lpx_min
    //{CAMERA_REG_OP_DATA,0x4825,0x01},
    {CAMERA_REG_OP_DATA,0x4826,0x05}, //hs_prepare_min
    //{CAMERA_REG_OP_DATA,0x4827,0x80}, //hs_prepare_max
#endif

    {CAMERA_REG_OP_DATA,0x5000,0x9f},
    {CAMERA_REG_OP_DATA,0x5001,0x00},
    {CAMERA_REG_OP_DATA,0x5e00,0x00},
    {CAMERA_REG_OP_DATA,0x5d00,0x07},
    {CAMERA_REG_OP_DATA,0x5d01,0x00},
    {CAMERA_REG_OP_DATA,0x4f00,0x04},
    {CAMERA_REG_OP_DATA,0x4f10,0x00},
    {CAMERA_REG_OP_DATA,0x4f11,0x98},
    {CAMERA_REG_OP_DATA,0x4f12,0x0f},
    {CAMERA_REG_OP_DATA,0x4f13,0xc4},
    //{CAMERA_REG_OP_DATA,0x0100,0x01}, //streaming
    {CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};



static struct tx_isp_sensor_win_setting ov9281_win_sizes[] = {
    {
        .width      = 640,
        .height     = 480,
        .fps        = 120 << 16 | 1,
        .mbus_code  = V4L2_MBUS_FMT_SGRBG10_1X10,
        .colorspace = V4L2_COLORSPACE_SRGB,
        .regs       = ov9281_init_regs_640_480_120fps_mipi,
    },
    {
        .width      = 1280,
        .height     = 720,
        .fps        = 60 << 16 | 1,
        .mbus_code  = V4L2_MBUS_FMT_SGRBG10_1X10,
        .colorspace = V4L2_COLORSPACE_SRGB,
        .regs       = ov9281_init_regs_1280_720_60fps_mipi,
    },
    {
        .width      = 640,
        .height     = 400,
        .fps        = 120 << 16 | 1,
        .mbus_code  = V4L2_MBUS_FMT_SGRBG10_1X10,
        .colorspace = V4L2_COLORSPACE_SRGB,
        .regs       = ov9281_init_regs_640_400_120fps_mipi,
    },
};

/*
 * the part of driver was fixed.
 */

static struct camera_reg_op ov9281_stream_on_mipi[] = {
    {CAMERA_REG_OP_DATA,0x0100,0x01},
    {CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};

static struct camera_reg_op ov9281_stream_off_mipi[] = {
    {CAMERA_REG_OP_DATA,0x0100,0x00},
    {CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};

int ov9281_read(struct tx_isp_subdev *sd, u16 reg, u8 *val)
{
    int ret;

    u8 buf[2] = {reg >> 8, reg & 0xff};

    struct i2c_client *client = tx_isp_get_subdevdata(sd);
    struct i2c_msg msg[2] = {
        [0] = {
            .addr   = client->addr,
            .flags  = 0,
            .len    = 2,
            .buf    = buf,
        },
        [1] = {
            .addr   = client->addr,
            .flags  = I2C_M_RD,
            .len    = 1,
            .buf    = val,
        }
    };

    ret = private_i2c_transfer(client->adapter, msg, 2);
    if (ret > 0)
        ret = 0;

    return ret;
}

int ov9281_write(struct tx_isp_subdev *sd, u16 reg, u8 val)
{
    struct i2c_client *client = tx_isp_get_subdevdata(sd);

    u8 buf[3] = {reg >> 8, reg & 0xff, val};
    struct i2c_msg msg = {
        .addr   = client->addr,
        .flags  = 0,
        .len    = 3,
        .buf    = buf,
    };
    int ret;
    ret = private_i2c_transfer(client->adapter, &msg, 1);
    if (ret > 0)
        ret = 0;

    return ret;
}

static int ov9281_read_array(struct tx_isp_subdev *sd, struct camera_reg_op *vals)
{
    int ret;
    u8 val;
    while (vals->flag != CAMERA_REG_OP_END) {
        if (vals->flag == CAMERA_REG_OP_DATA) {
            ret = ov9281_read(sd, vals->reg, &val);
            if (ret < 0)
                return ret;
            printk("%x[%x]\n", vals->reg, val);
        } else if (vals->flag == CAMERA_REG_OP_DELAY) {
            private_msleep(vals->val);
        } else {
            pr_debug("%s(%d), error flag: %d\n", __func__, __LINE__, vals->flag);
            return -1;
        }
        vals++;
    }

    return 0;
}
static int ov9281_write_array(struct tx_isp_subdev *sd, struct camera_reg_op *vals)
{
    int ret;
    while (vals->flag != CAMERA_REG_OP_END) {
        if (vals->flag == CAMERA_REG_OP_DATA) {
            ret = ov9281_write(sd, vals->reg, vals->val);
            if (ret < 0)
                return ret;
        } else if (vals->flag == CAMERA_REG_OP_DELAY) {
            private_msleep(vals->val);
        } else {
            pr_debug("%s(%d), error flag: %d\n", __func__, __LINE__, vals->flag);
            return -1;
        }
        vals++;
    }

    return 0;
}

static int ov9281_reset(struct tx_isp_subdev *sd, int val)
{
    return 0;
}

static int ov9281_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
    unsigned char v;
    int ret;

    ret = ov9281_read(sd, 0x300A, &v);
    pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
    if (ret < 0)
        return ret;
    if (v != OV9281_CHIP_ID_H)
        return -ENODEV;
    *ident = v;

    ret = ov9281_read(sd, 0x300B, &v);
    pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
    if (ret < 0)
        return ret;
    if (v != OV9281_CHIP_ID_L)
        return -ENODEV;

    *ident = (*ident << 8) | v;

    return 0;
}

static int ov9281_set_integration_time(struct tx_isp_subdev *sd, int value)
{
    int ret = 0;
    unsigned int expo = value;
    unsigned char tmp;
    unsigned short vts;
    unsigned int max_expo;

    ret = ov9281_read(sd, 0x380e, &tmp);
    vts = tmp << 8;
    ret += ov9281_read(sd, 0x380f, &tmp);
    if(ret < 0)
        return ret;
    vts |= tmp;

    max_expo = vts - 25;
    if (expo > max_expo)
        expo = max_expo;

    //0x3500, [19:16]
    //0x3501, [15:8]
    //0x3502, [7:0], low 4 bits are fraction bits
    ret  = ov9281_write(sd, 0x3500, (unsigned char)((expo & 0xffff) >> 12));
    ret += ov9281_write(sd, 0x3501, (unsigned char)((expo & 0x0fff) >> 4));
    ret += ov9281_write(sd, 0x3502, (unsigned char)((expo & 0x000f) << 4));
    if (ret < 0) {
        printk("ov9281_write error  %d\n" ,__LINE__ );
        return ret;
    }
    return 0;
}

/**************************************
  ov9281 sensor gain control by [0x3509]
    value from 0x10 to 0xF8 represents gain of 1x to 15.5x
**************************************/
static int ov9281_set_analog_gain(struct tx_isp_subdev *sd, u16 gain)
{
    int ret = 0;

    ret = ov9281_write(sd, 0x3509, gain);
    if (ret < 0) {
        printk("ov9281_write error  %d" ,__LINE__ );
        return ret;
    }

    return 0;
}

static int ov9281_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
    return 0;
}

static int ov9281_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
    return 0;
}

static int ov9281_init(struct tx_isp_subdev *sd, int enable)
{
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    struct tx_isp_sensor_win_setting *wsize;
    int ret = 0;

    if(!enable)
        return ISP_SUCCESS;

#ifdef OV9281_720P_SUPPORT
    wsize = &ov9281_win_sizes[1];
#else
    wsize = &ov9281_win_sizes[0];
#endif

    sensor->video.mbus.width = wsize->width;
    sensor->video.mbus.height = wsize->height;
    sensor->video.mbus.code = wsize->mbus_code;
    sensor->video.mbus.field = V4L2_FIELD_NONE;
    sensor->video.mbus.colorspace = wsize->colorspace;
    sensor->video.fps = wsize->fps;
    ret = ov9281_write_array(sd, wsize->regs);
    if (ret)
        return ret;
    //ov9281_read_array(sd, wsize->regs);
    ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
    sensor->priv = wsize;

    return 0;
}

static int ov9281_s_stream(struct tx_isp_subdev *sd, int enable)
{
    int ret = 0;

    if (enable) {
        ret = ov9281_write_array(sd, ov9281_stream_on_mipi);
        pr_debug("ov9281 stream on\n");

    }
    else {
        ret = ov9281_write_array(sd, ov9281_stream_off_mipi);
        pr_debug("ov9281 stream off\n");
    }

    return ret;
}

static int ov9281_set_fps(struct tx_isp_subdev *sd, int fps)
{
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    unsigned int sclk = 0;
    unsigned short vts = 0;
    unsigned short hts=0;
    unsigned int max_fps = 0;
    unsigned char tmp;
    unsigned int newformat = 0; //the format is 24.8
    int ret = 0;

    sclk = OV9281_SCLK;
    max_fps = SENSOR_OUTPUT_MAX_FPS;

    /* the format of fps is 16/16. for example 25 << 16 | 2, the value is 25/2 fps. */
    newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
    if(newformat > (max_fps << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)){
        printk("warn: fps(%x) no in range\n", fps);
        return -1;
    }

    //HTS
    ret = ov9281_read(sd, 0x380c, &tmp);
    hts = tmp << 8;
    ret += ov9281_read(sd, 0x380d, &tmp);
    if(ret < 0)
        return -1;
    hts |= tmp;

    //VTS
    vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);

    ret = ov9281_write(sd, 0x380e, (unsigned char)(vts >> 8));
    ret += ov9281_write(sd, 0x380f, (unsigned char)(vts & 0xff));
    if(ret < 0)
        return -1;

    sensor->video.fps = fps;
    sensor->video.attr->max_integration_time_native = vts - 4;
    sensor->video.attr->integration_time_limit = vts - 4;
    sensor->video.attr->total_height = vts;
    sensor->video.attr->max_integration_time = vts - 4;
    ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

    return ret;
}

static int ov9281_set_mode(struct tx_isp_subdev *sd, int value)
{
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    struct tx_isp_sensor_win_setting *wsize = NULL;
    int ret = ISP_SUCCESS;

#ifdef OV9281_720P_SUPPORT
    if(value == TX_ISP_SENSOR_FULL_RES_MAX_FPS){
        wsize = &ov9281_win_sizes[1];
    }else if(value == TX_ISP_SENSOR_PREVIEW_RES_MAX_FPS){
        wsize = &ov9281_win_sizes[1];
    }
#else
    if(value == TX_ISP_SENSOR_FULL_RES_MAX_FPS){
        wsize = &ov9281_win_sizes[0];
    }else if(value == TX_ISP_SENSOR_PREVIEW_RES_MAX_FPS){
        wsize = &ov9281_win_sizes[0];
    }
#endif

    if(wsize){
        sensor->video.mbus.width = wsize->width;
        sensor->video.mbus.height = wsize->height;
        sensor->video.mbus.code = wsize->mbus_code;
        sensor->video.mbus.field = V4L2_FIELD_NONE;
        sensor->video.mbus.colorspace = wsize->colorspace;
        sensor->video.fps = wsize->fps;
        ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
    }

    return ret;
}

static int ov9281_set_vflip(struct tx_isp_subdev *sd, int enable)
{
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    int ret = 0;
    u8 val = 0;

    ret = ov9281_read(sd, 0x3820, &val);
    if (enable){
        val |= (1<<2);
        //sensor->video.mbus.code = V4L2_MBUS_FMT_SBGGR10_1X10;
    } else {
        val &= (1<<2);
        //sensor->video.mbus.code = V4L2_MBUS_FMT_SGRBG10_1X10;
    }
    ret += ov9281_write(sd, 0x3820, val);

    if(!ret)
        ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

    return ret;
}

static int ov9281_g_chip_ident(struct tx_isp_subdev *sd,
        struct tx_isp_chip_ident *chip)
{
    struct i2c_client *client = tx_isp_get_subdevdata(sd);
    unsigned int ident = 0;
    int ret = ISP_SUCCESS;

    if(dvdd_en_gpio != -1){
        ret = private_gpio_request(dvdd_en_gpio,"ov9281_dvdd_en");
        if(!ret){
            private_gpio_direction_output(dvdd_en_gpio, 1);
            private_msleep(20);
        }else{
            printk("gpio requrest fail %d\n",pwdn_gpio);
        }
    }

    if(pwdn_gpio != -1){
        ret = private_gpio_request(pwdn_gpio,"ov9281_pwdn");
        if(!ret){
            private_gpio_direction_output(pwdn_gpio, 0);
            private_msleep(10);
            private_gpio_direction_output(pwdn_gpio, 1);
            private_msleep(50);
        }else{
            printk("gpio requrest fail %d\n",pwdn_gpio);
        }
    }

    ret = ov9281_detect(sd, &ident);
    if (ret) {
        printk("chip found @ 0x%x (%s) is not an ov9281 chip.\n",
                client->addr, client->adapter->name);
        return ret;
    }
    printk("ov9281 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
    if(chip){
        memcpy(chip->name, "ov9281", sizeof("ov9281"));
        chip->ident = ident;
        chip->revision = SENSOR_VERSION;
    }

    return 0;
}

static int ov9281_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
    long ret = 0;

    if(IS_ERR_OR_NULL(sd)){
        printk("[%d]The pointer is invalid!\n", __LINE__);
        return -EINVAL;
    }
    switch(cmd){
    case TX_ISP_EVENT_SENSOR_INT_TIME:
        if(arg)
            ret = ov9281_set_integration_time(sd, *(int*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_AGAIN:
        if(arg)
            ret = ov9281_set_analog_gain(sd, *(u16*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_DGAIN:
        if(arg)
            ret = ov9281_set_digital_gain(sd, *(int*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
        if(arg)
            ret = ov9281_get_black_pedestal(sd, *(int*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_RESIZE:
        if(arg)
            ret = ov9281_set_mode(sd, *(int*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
        ret = ov9281_write_array(sd, ov9281_stream_off_mipi);
        break;
    case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
        ret = ov9281_write_array(sd, ov9281_stream_on_mipi);
        break;
    case TX_ISP_EVENT_SENSOR_FPS:
        if(arg)
            ret = ov9281_set_fps(sd, *(int*)arg);
        break;
    case TX_ISP_EVENT_SENSOR_VFLIP:
        if(arg)
            ret = ov9281_set_vflip(sd, *(int*)arg);
        break;
    default:
        break;;
    }

    return 0;
}

static int ov9281_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
{
    u8 val = 0;
    int len = 0;
    int ret = 0;

    len = strlen(sd->chip.name);
    if(len && strncmp(sd->chip.name, reg->name, len)){
        return -EINVAL;
    }
    if (!private_capable(CAP_SYS_ADMIN))
        return -EPERM;
    ret = ov9281_read(sd, reg->reg & 0xffff, &val);
    reg->val = val;
    reg->size = 1;

    return ret;
}

static int ov9281_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
    int len = 0;

    len = strlen(sd->chip.name);
    if(len && strncmp(sd->chip.name, reg->name, len)){
        return -EINVAL;
    }
    if (!private_capable(CAP_SYS_ADMIN))
        return -EPERM;
    ov9281_write(sd, reg->reg & 0xffff, reg->val & 0xff);

    return 0;
}

static struct tx_isp_subdev_core_ops ov9281_core_ops = {
    .g_chip_ident = ov9281_g_chip_ident,
    .reset = ov9281_reset,
    .init = ov9281_init,
    /*.ioctl = ov9281_ops_ioctl,*/
    .g_register = ov9281_g_register,
    .s_register = ov9281_s_register,
};

static struct tx_isp_subdev_video_ops ov9281_video_ops = {
    .s_stream = ov9281_s_stream,
};

static struct tx_isp_subdev_sensor_ops ov9281_sensor_ops = {
    .ioctl = ov9281_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops ov9281_ops = {
    .core = &ov9281_core_ops,
    .video = &ov9281_video_ops,
    .sensor = &ov9281_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
    .name = "ov9281",
    .id = -1,
    .dev = {
        .dma_mask = &tx_isp_module_dma_mask,
        .coherent_dma_mask = 0xffffffff,
        .platform_data = NULL,
    },
    .num_resources = 0,
};

static int ov9281_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct tx_isp_subdev *sd;
    struct tx_isp_video_in *video;
    struct tx_isp_sensor *sensor;
    struct tx_isp_sensor_win_setting *wsize;

    pr_debug("probe ok ----start--->ov9281\n");
    sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
    if(!sensor){
        printk("Failed to allocate sensor subdev.\n");
        return -ENOMEM;
    }
    memset(sensor, 0 ,sizeof(*sensor));
    /* request mclk of sensor */
    sensor->mclk = clk_get(NULL, "cgu_cim");
    if (IS_ERR(sensor->mclk)) {
        printk("Cannot get sensor input clock cgu_cim\n");
        goto err_get_mclk;
    }
    private_clk_set_rate(sensor->mclk, 24000000);
    private_clk_enable(sensor->mclk);

#ifdef OV9281_720P_SUPPORT
    wsize = &ov9281_win_sizes[1];
#else
    wsize = &ov9281_win_sizes[0];
#endif

    sd = &sensor->sd;
    video = &sensor->video;
    sensor->video.attr = &ov9281_attr;
    sensor->video.vi_max_width = wsize->width;
    sensor->video.vi_max_height = wsize->height;
    sensor->video.mbus.width = wsize->width;
    sensor->video.mbus.height = wsize->height;
    sensor->video.mbus.code = wsize->mbus_code;
    sensor->video.mbus.field = V4L2_FIELD_NONE;
    sensor->video.mbus.colorspace = wsize->colorspace;
    sensor->video.fps = wsize->fps;
    tx_isp_subdev_init(&sensor_platform_device, sd, &ov9281_ops);
    tx_isp_set_subdevdata(sd, client);
    tx_isp_set_subdev_hostdata(sd, sensor);
    private_i2c_set_clientdata(client, sd);

    pr_debug("probe ok ------->ov9281\n");

    return 0;

#if 0
    private_clk_disable(sensor->mclk);
    private_clk_put(sensor->mclk);
#endif
    err_get_mclk:
    kfree(sensor);

    return -1;
}

static int ov9281_remove(struct i2c_client *client)
{
    struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
    struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

    if(dvdd_en_gpio != -1)
        private_gpio_free(dvdd_en_gpio);

    if(pwdn_gpio != -1)
        private_gpio_free(pwdn_gpio);

    private_clk_disable(sensor->mclk);
    private_clk_put(sensor->mclk);
    tx_isp_subdev_deinit(sd);
    kfree(sensor);

    return 0;
}

static const struct i2c_device_id ov9281_id[] = {
    { "ov9281", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ov9281_id);

static struct i2c_driver ov9281_driver = {
    .driver = {
        .owner  = THIS_MODULE,
        .name   = "ov9281",
    },
    .probe      = ov9281_probe,
    .remove     = ov9281_remove,
    .id_table   = ov9281_id,
};

static __init int init_ov9281(void)
{
    int ret = 0;
    ret = private_driver_get_interface();
    if(ret){
        printk("Failed to init ov9281 dirver.\n");
        return -1;
    }

    return private_i2c_add_driver(&ov9281_driver);
}

static __exit void exit_ov9281(void)
{
    private_i2c_del_driver(&ov9281_driver);
}

module_init(init_ov9281);
module_exit(exit_ov9281);

MODULE_DESCRIPTION("A low-level driver for Gcoreinc ov9281 sensors");
MODULE_LICENSE("GPL");

