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

typedef void (*alarm_listener_t)(void);

struct alarm_manager {
    int (*init)(void);
    int (*deinit)(void);
    int (*start)(void);
    int (*stop)(void);
    int (*is_start)(void);
    void (*set)(uint64_t when, alarm_listener_t listener);
    void (*cancel)(alarm_listener_t listener);
    uint64_t (*get_sys_time_ms)(void);
};

struct alarm_manager* get_alarm_manager(void);
