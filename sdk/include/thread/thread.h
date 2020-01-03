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

#ifndef THREAD_H
#define THREAD_H

#include <thread/runnable.h>
#include <thread/pthread_wrapper.h>

struct thread {
    struct runnable runnable;

    void (*construct)(struct thread* this);
    void (*destruct)(struct thread* this);

    void (*set_thread_count)(struct thread* this, int count);
    int (*start)(struct thread* this, void* param);
    void (*stop)(struct thread* this);
    int (*is_running)(struct thread* this);
    void (*wait)(struct thread* this);

    int pthread_count;
    struct pthread_wrapper* pthreads;
};

void construct_thread(struct thread* this);
void destruct_thread(struct thread* this);

#endif /* THREAD_H */
