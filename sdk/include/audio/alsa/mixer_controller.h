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

#ifndef MIXER_CONTROLLER_H
#define MIXER_CONTROLLER_H

enum {
    PLAYBACK,
    CAPTURE
};

struct mixer_controller {
    int (*init)(void);
    int (*deinit)(void);
    int (*set_volume)(const char* name, int dir, int volume);
    int (*get_volume)(const char* name, int dir);
    int (*mute)(const char* name, int mute);
};

struct mixer_controller* get_mixer_controller(void);

#endif /* MIXER_CONTROLLER_H */
