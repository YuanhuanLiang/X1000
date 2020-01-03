/*
 *  Copyright (C) 2016, Zhang YanMing <jamincheung@126.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef GR_DRAWER_H
#define GR_DRAWER_H

#include <types.h>

struct gr_surface {
    uint32_t width;
    uint32_t height;
    uint32_t row_bytes;
    uint32_t pixel_bytes;
    uint8_t *raw_data;
};

struct gr_drawer {
    int (*init)(void);
    int (*deinit)(void);

    uint32_t (*get_fb_width)(void);
    uint32_t (*get_fb_height)(void);

    void (*get_font_size)(uint32_t *width, uint32_t* height);

    void (*set_pen_color)(uint8_t red, uint8_t green,
            uint8_t blue);

    int (*draw_png)(struct gr_surface* surface, uint32_t pos_x, uint32_t pos_y);

    //surface(pos_x,pos_y) background(bpos_x, bpos_y)
    int (*draw_png_alpha)(struct gr_surface* surface, struct gr_surface* background, uint32_t pos_x, uint32_t pos_y, uint32_t bpos_x, uint32_t bpos_y);

    int (*draw_text)(uint32_t pos_x, uint32_t pos_y, const char* text, uint8_t bold);

    void (*display)(void);

    int (*blank)(uint8_t blank);
    void (*fill_screen)(void);
    int (*fill_rect)(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
};

struct gr_drawer* get_gr_drawer(void);

#endif /* GR_DRAWER_H */
