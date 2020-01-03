#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <utils/log.h>
#include <utils/common.h>
#include <fb/fb_manager.h>

#define LOG_TAG "test_fb"

static struct fb_manager* fb_manager;

static inline uint32_t  make_pixel(uint32_t r, uint32_t g, uint32_t b)
{
    return (uint32_t)(((r >> 3) << 11) | ((g >> 2) << 5 | (b >> 3)));
}

static void fill_pixel(uint32_t pixel, uint32_t x0, uint32_t y0, uint32_t w,
        uint32_t h)
{
    int i, j;
    uint16_t *pbuf = (uint16_t *)fb_manager->get_fbmem();
    for (i = y0; i < h; i ++) {
        for (j = x0; j < w; j ++) {
            pbuf[i * w + j] = pixel;
        }
    }
}

int main(int argc, char *argv[]) {
    fb_manager = get_fb_manager();

    if (fb_manager->init() < 0) {
        LOGE("Failed to init fb_manager\n");
        return -1;
    }

    fb_manager->dump();

    uint32_t red_pixel = make_pixel(0xff, 0, 0);
    uint32_t green_pixel = make_pixel(0, 0xff, 0);
    uint32_t blue_pixel = make_pixel(0, 0, 0xff);

    for (;;) {

        fill_pixel(red_pixel, 0, 0, fb_manager->get_screen_height(),
                fb_manager->get_screen_width());
        fb_manager->display();

        sleep(1);

        fill_pixel(green_pixel, 0, 0, fb_manager->get_screen_height(),
                fb_manager->get_screen_width());
        fb_manager->display();

        sleep(1);

        fill_pixel(blue_pixel, 0, 0, fb_manager->get_screen_height(),
                fb_manager->get_screen_width());
        fb_manager->display();

        sleep(1);
    }

    return 0;
}
