#ifndef __PIXEL_H__
#define __PIXEL_H__
/*
struct bpp16_pixel {
    unsigned short red:5;
    unsigned short green:6;
    unsigned short blue:5;
};
*/
struct bpp16_pixel {
    unsigned short blue:5;
    unsigned short green:6;
    unsigned short red:5;
};

struct bpp24_pixel {
    __u8 red;
    __u8 green;
    __u8 blue;
};

struct bpp32_pixel {
    __u8 red;
    __u8 green;
    __u8 blue;
    __u8 alpha;
};

struct yuv422_sample {
    __u8 b1;
    __u8 b2;
    __u8 b3;
    __u8 b4;
};

struct yuv444_sample {
    __u8 b1;
    __u8 b2;
    __u8 b3;
};

struct rgb888_sample {
    __u8 b1;
    __u8 b2;
    __u8 b3;
};

struct rgb565_sample {
    unsigned short b1:5,
        b2:6,
        b3:5;
};

#endif /* __PIXEL_H__ */
