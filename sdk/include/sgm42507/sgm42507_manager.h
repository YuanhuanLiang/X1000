/*
 *  Copyright (C) 2017, Monk Su<rongjin.su@ingenic.com, MonkSu@outlook.com>
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
#ifndef _SGM42507_MANAGER_H_
#define _SGM42507_MANAGER_H_



typedef enum {
    SGM42507_POWER_OFF = 0,
    SGM42507_POWER_ON
}onoff_m;

typedef enum {
    SGM42507_DRI_DISABLE = 0,
    SGM42507_DRI_ENABLE
}en_m;

typedef enum {
    SGM42507_DIRECTION_A = 0,
    SGM42507_DIRECTION_B
}dir_m;

struct sgm42507_manager{
    int (*init)(void);
    int (*deinit)(void);
    int (*set_power)(onoff_m onoff);
    int (*drv_enable)(en_m en);
    int (*set_direction)(dir_m dir);
};

struct sgm42507_manager* get_sgm42507_manager();
#endif