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
#ifndef _FXL_LOCK_MANAGER_H_
#define _FXL_LOCK_MANAGER_H_

typedef enum{
    FXL_STRETCH,
    FXL_SHRINK,
    FXL_SHAKE,
}fxl_tongue_m;

typedef struct _fxl_tongue_sta_t {
    fxl_tongue_m bolique_tongue;
    fxl_tongue_m square_tongue;
}fxl_tongue_sta_t;

typedef void (*fxl_lock_body_status_cb)(fxl_tongue_sta_t tongue);


struct fxl_lock_body_manager{
    int (*init)(fxl_lock_body_status_cb cb);
    int (*get_status)(fxl_tongue_sta_t* tongue);
    void (*deinit)(void);
};


struct fxl_lock_body_manager* get_fxl_lock_body_manager();
#endif