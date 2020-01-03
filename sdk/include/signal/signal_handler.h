/*
 *  Copyright (C) 2016, Zhang YanMing <jamincheung@126.com>
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

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

typedef void(*signal_handler_t)(int signal);

struct signal_handler {
    void (*construct)(struct signal_handler *this);
    void (*destruct)(struct signal_handler *this);
    void (*set_signal_handler) (struct signal_handler* this,
            int signal, signal_handler_t handler);

    struct sigaction action;
};

void construct_signal_handler(struct signal_handler *this);
void destruct_signal_handler(struct signal_handler* this);
#endif /* SIGNAL_HANDLER_H */
