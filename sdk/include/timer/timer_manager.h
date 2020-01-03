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

#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#define TIMER_MAX_COUNT     64

typedef void (*timer_event_listener_t)(int timer_id, int exp_num);

struct timer_manager {
    int (*init)(void);
    int (*deinit)(void);
    int (*is_init)(void);
    int (*alloc_timer)(int init_exp, int interval, int oneshot,
            timer_event_listener_t listener);
    int (*free_timer)(int timer_id);
    uint64_t (*remain_ms)(int timer_id);
    int (*enumerate)(int* id_table);
    int (*start)(int timer_id);
    int (*stop)(int timer_id);
    int (*is_start)(int timer_id);
};

struct timer_manager* get_timer_manager(void);

#endif /* TIMER_MANAGER_H */
