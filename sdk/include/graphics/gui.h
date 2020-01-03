/*
 *  Copyright (C) 2016, Zhang YanMing <jamincheung@126.com>
 *
 *  Linux recovery updater
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

#ifndef GUI_H
#define GUI_H

#include <types.h>
#include <graphics/gr_drawer.h>

enum update_stage_t {
    UPDATING,
    UPDATE_SUCCESS,
    UPDATE_FAILURE
};

struct gui {
    void (*construct)(struct gui* this);
    void (*destruct)(struct gui* this);

    int (*init)(struct gui* this);
    int (*deinit)(struct gui* this);

    int (*show_log)(struct gui* this, const char* fmt, ...);
    int (*start_show_progress)(struct gui* this);
    int (*stop_show_progress)(struct gui* this);
    int (*show_logo)(struct gui* this, uint32_t pos_x, uint32_t pos_y);
    int (*show_tips)(struct gui* this, enum update_stage_t stage);
    void (*clear)(struct gui* this);
};

void construct_gui(struct gui* this);
void destruct_gui(struct gui* this);

#endif /* GUI_H */
