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

#ifndef NETLINK_EVENT_H
#define NETLINK_EVENT_H

#define NL_PARAMS_MAX       128

#define NLACTION_UNKNOWN    0
#define NLACTION_ADD        1
#define NLACTION_REMOVE     2
#define NLACTION_CHANGE     3
#define NLACTION_LINKUP     4
#define NLACTION_LINKDOWN   5

struct netlink_event {
    void (*construct)(struct netlink_event *this);
    void (*destruct)(struct netlink_event *this);
    int (*decode)(struct netlink_event *this, char *buffer, int size,
            int format);
    const char *(*find_param)(struct netlink_event *this,
            const char *param_name);
    const char *(*get_subsystem)(struct netlink_event* this);
    const int (*get_action)(struct netlink_event *this);
    void (*dump)(struct netlink_event* this);
    int seq;
    char *path;
    int action;
    char *subsystem;
    char *params[NL_PARAMS_MAX];
};

void construct_netlink_event(struct netlink_event* this);
void destruct_netlink_event(struct netlink_event* this);

#endif /* NETLINK_EVENT_H */
