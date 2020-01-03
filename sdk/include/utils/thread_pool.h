/*
 *  Copyright (C) 2016, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
 *
 *  Ingenic QRcode SDK Project
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

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <limits.h>

typedef void (*func_thread_async) (void *arg);
typedef void (*func_thread_handle) (void *arg);
struct thread_work {
    func_thread_handle routine;
    void *arg;
    func_thread_async async;
    void *async_arg;
    struct list_head  list_cell;
};

struct thread_pool {
    pthread_t *thread_id;
    uint32_t max_thread_cnt;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;
    struct thread_work *queue_head;
    struct thread_work *queue_end;
    uint32_t queue_size;
    uint8_t stop;
    uint8_t exit;
    struct list_head work_head;
};

struct thread_pool_manager {
    int32_t (*init)(struct thread_pool_manager* pm,
            uint32_t thread_cnt, int32_t policy, int32_t priority);
    int32_t (*destroy)(struct thread_pool_manager* pm, uint32_t thread_cnt);
    int32_t (*start)(struct thread_pool_manager* pm);
    int32_t (*stop)(struct thread_pool_manager* pm);
    int32_t (*add_work)(struct thread_pool_manager* pm, func_thread_handle routine,
            void *arg,  func_thread_async async, void *async_arg);
    struct thread_pool *pool;
};

int32_t thread_set_priority(pthread_attr_t *attr, int policy, int priority);
// Interface exported as below
struct thread_pool_manager* get_thread_pool_manager(void);
struct thread_pool_manager* construct_thread_pool_manager(void);
int32_t deconstruct_thread_pool_manager(struct thread_pool_manager** manager);

#endif