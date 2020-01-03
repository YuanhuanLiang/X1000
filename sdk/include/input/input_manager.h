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

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <linux/input.h>

typedef void (*input_event_listener_t)(const char* input_name,
        struct input_event *event);

struct input_manager {
    int (*init)(void);
    int (*deinit)(void);
    int (*start)(void);
    int (*is_start)(void);
    int (*stop)(void);
    int (*get_devices_count)(void);
    void (*register_event_listener)(const char* name,
            input_event_listener_t listener);
    void (*unregister_event_listener)(const char*name, input_event_listener_t listener);
    void (*dump_event)(struct input_event* event);
    const char* (*type2str)(uint32_t event_type);
    const char* (*code2str)(uint32_t event_type, uint32_t event_code);
};

struct input_manager* get_input_manager(void);

#endif /* INPUT_MANAGER_H */
