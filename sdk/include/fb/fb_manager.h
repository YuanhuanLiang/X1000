/*
 *  Copyright (C) 2017, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
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

#ifndef FB_MANAGER_H
#define FB_MANAGER_H

#include <types.h>
#include <linux/fb.h>

struct fb_manager {
    int (*init)(void);
    int (*deinit)(void);
    void (*dump)(void);
    void (*display)(void);
    int (*blank)(uint8_t blank);

    uint8_t* (*get_fbmem)(void);
    uint32_t (*get_screen_size)(void);
    uint32_t (*get_screen_width)(void);
    uint32_t (*get_screen_height)(void);

    uint32_t (*get_redbit_offset)(void);
    uint32_t (*get_redbit_length)(void);

    uint32_t (*get_greenbit_offset)(void);
    uint32_t (*get_greenbit_length)(void);

    uint32_t (*get_bluebit_offset)(void);
    uint32_t (*get_bluebit_length)(void);

    uint32_t (*get_alphabit_offset)(void);
    uint32_t (*get_alphabit_length)(void);

    uint32_t (*get_bits_per_pixel)(void);
    uint32_t (*get_row_bytes)(void);
};

struct fb_manager* get_fb_manager(void);

#endif /* FB_MANAGER_H */
