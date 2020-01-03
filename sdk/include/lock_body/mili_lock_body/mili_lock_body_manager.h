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
#ifndef _MILI_LOCK_MANAGER_H_
#define _MILI_LOCK_MANAGER_H_

typedef enum{
    MILI_STRETCH,
    MILI_SHRINK,
    MILI_SHAKE,
    MILI_ROTATE,
    MILI_UNROTATE,
}mili_tongue_m;

typedef struct _mili_tongue_sta_t {
    mili_tongue_m master_square;
    mili_tongue_m anti_square;
    mili_tongue_m lock_cylinder;
}mili_tongue_sta_t;

typedef void (*mili_lock_body_status_cb)(mili_tongue_sta_t tongue);


struct mili_lock_body_manager{
    int (*init)(mili_lock_body_status_cb cb);
    int (*get_status)(mili_tongue_sta_t* tongue);
    void (*deinit)(void);
};


struct mili_lock_body_manager* get_mili_lock_body_manager();
#endif