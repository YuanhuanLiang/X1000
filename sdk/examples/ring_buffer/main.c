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

#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <thread/thread.h>
#include <ring_buffer/ring_buffer.h>

#define LOG_TAG "test_ring_buffer"

#define BUF_SIZE 64
#define PUT_SIZE 33
#define GET_SIZE 25

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static struct ring_buffer* rb;
static struct thread* runner;
static struct signal_handler* signal_handler;
static char buffer[BUF_SIZE];

static void thread_loop(struct pthread_wrapper* thread, void *param) {
    char rd_buf[GET_SIZE] = {0};
    int offset = 0;
    int count = 0;

    for (;;) {
        offset = 0;
        count = 0;

repop:
        msleep(300);

        pthread_mutex_lock(&lock);

        LOGI("================Read==================\n");
        if (rb->empty(rb)) {
            LOGW("Underrun: ring buffer is empty\n");
            pthread_mutex_unlock(&lock);
            LOGI("======================================\n");
            continue;
        }

        if (rb->get_available_size(rb) < ARRAY_SIZE(rd_buf)) {
            LOGW("Underrun: ring buffer is not enough\n");
            pthread_mutex_unlock(&lock);
            LOGI("======================================\n");
            continue;
        }

        count = rb->pop(rb, rd_buf + offset, ARRAY_SIZE(rd_buf) - offset);

        for (int i = 0; i < ARRAY_SIZE(rd_buf); i++)
            LOGI("readed[%d]=0x%x\n", i, rd_buf[i]);

        LOGI("======================================\n");

        pthread_mutex_unlock(&lock);

        offset += count;
        if (offset < ARRAY_SIZE(rd_buf))
            goto repop;

    }

    pthread_exit(NULL);
}

static void thread_exit(void) {
    LOGI("Thread not need cleanup here\n");
}

static void handle_signal(int signal) {
    runner->stop(runner);

    _delete(rb);

    exit(1);
}

static void init_buffer(void) {
    for (int i = 0; i < ARRAY_SIZE(buffer); i++)
        buffer[i] = i;
}

int main(int argc, char *argv[]) {
    int offset = 0;
    int count = 0;
    char wr_buf[PUT_SIZE] = {0};
    int index = 0;

    rb = _new(struct ring_buffer, ring_buffer);
    runner = _new(struct thread, thread);
    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    rb->set_capacity(rb, BUF_SIZE);
    runner->runnable.run = thread_loop;
    runner->runnable.cleanup = thread_exit;

    init_buffer();

    runner->start(runner, NULL);

    while (1) {
        for (int i = 0; i < ARRAY_SIZE(wr_buf); i++) {
            wr_buf[i] = buffer[index];
            if (++index == ARRAY_SIZE(buffer))
                index = 0;
        }

        offset = 0;
        count = 0;

repush:
        msleep(200);

        pthread_mutex_lock(&lock);

        LOGI("================Write==================\n");

        if (rb->full(rb)) {
            LOGW("Overrun: ring buffer is full\n");
            pthread_mutex_unlock(&lock);
            LOGI("======================================\n");
            goto repush;
        }

        count = rb->push(rb, wr_buf + offset, ARRAY_SIZE(wr_buf) - offset);
        if (count < 0) {
            LOGE("Failed to push buffer to ring buffer\n");
            pthread_mutex_unlock(&lock);
            return -1;
        }

        for (int i = offset; i < ARRAY_SIZE(wr_buf); i++)
            LOGI("writed[%d]=0x%x\n", i, wr_buf[i]);

        LOGI("=======================================\n");

        pthread_mutex_unlock(&lock);

        offset += count;
        if (offset < ARRAY_SIZE(wr_buf))
            goto repush;
    }

    return 0;
}
