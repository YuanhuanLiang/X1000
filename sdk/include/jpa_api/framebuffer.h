#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

struct fb_info {
    int fd;
    int xres;
    int yres;
    int bpp;

    char dev_name[32];

    __u32 xoff;
    __u32 yoff;

    int smem_len;
    __u8 *fb_mem;
    int padding[2];
};

#endif /* __FRAMEBUFFER_H__ */
