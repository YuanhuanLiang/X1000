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

#ifndef PTHREAD_WRAPPER_H
#define PTHREAD_WRAPPER_H

#include <pthread.h>
#include <thread/runnable.h>

struct pthread_wrapper {
    void (*construct)(struct pthread_wrapper* this);
    void (*destruct)(struct pthread_wrapper* this);
    int (*start)(struct pthread_wrapper* this, struct runnable* runnable,
            void* param);
    void (*join)(struct pthread_wrapper* this);
    void (*cancel)(struct pthread_wrapper* this);
    struct runnable* (*get_runnable)(struct pthread_wrapper* this);
    void* (*get_param)(struct pthread_wrapper* this);
    int (*get_pid)(struct pthread_wrapper* this);
    int (*is_running)(struct pthread_wrapper* this);

    pid_t pid;
    pthread_t tid;
    struct runnable* runnable;
    int is_start;
    void* param;
};

void construct_pthread_wrapper(struct pthread_wrapper* this);
void destruct_pthread_wrapper(struct pthread_wrapper* this);

#endif /* PTHREAD_WRAPPER_H */
