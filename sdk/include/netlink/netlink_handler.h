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

#ifndef NETLINK_HANDLER_H
#define NETLINK_HANDLER_H

#include <netlink/netlink_event.h>

struct netlink_handler {
    void (*construct)(struct netlink_handler *this, char* subsystem,
            int priority,
            void (*handle_event)(struct netlink_handler* this,
                    struct netlink_event* event), void* param);
    void (*deconstruct)(struct netlink_handler *this);
    char* (*get_subsystem)(struct netlink_handler* this);
    int (*get_priority)(struct netlink_handler* this);
    void (*handle_event)(struct netlink_handler* this,
            struct netlink_event* event);
    void* (*get_private_data)(struct netlink_handler* this);
    char* subsystem;
    int priority;
    void* private_data;
    struct netlink_handler *next;
};

void construct_netlink_handler(struct netlink_handler* this, char* subsystem,
        int priority,
        void (*handle_event)(struct netlink_handler* this,
                struct netlink_event* event), void* param);
void destruct_netlink_handler(struct netlink_handler* this);

#endif /* NETLINK_HANDLER_H */
