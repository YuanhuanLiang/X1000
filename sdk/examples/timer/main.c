#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <timer/timer_manager.h>

#define LOG_TAG "test_timer"

static struct timer_manager* timer_manager;
static struct signal_handler* signal_handler;

struct timer_id {
    int id;
    int tot_exp;
};

static struct timer_id timer_array[3];

#define TIMER_INTERVAL_MS  500
#define TIMER_INIT_EXP_MS  2000
#define TIMER_ONESHOT       0

static void handle_signal(int signal) {
    int error = 0;
    int temp_array[TIMER_MAX_COUNT];
    int size = 0;

    size = timer_manager->enumerate(temp_array);
    if (size < 0) {
        LOGE("Failed to enumerate timers\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        LOGI("Going to stop timer(%d)\n", temp_array[i]);
        if (timer_manager->is_start(temp_array[i])) {
            error = timer_manager->stop(temp_array[i]);
            if (error < 0) {
                LOGE("Failed to stop timer(%d)\n", temp_array[i]);
                return;
            }

            error = timer_manager->free_timer(temp_array[i]);
            if (error < 0) {
                LOGE("Failed to free timer(%d)\n", temp_array[i]);
                return;
            }
        }
    }

    error = timer_manager->deinit();
    if (error < 0)
        LOGE("Failed to deinit timer manager\n");

    exit(1);
}

static void timer_event_listener(int timer_id, int exp_num) {
    LOGI("==============================================================\n");
    for (int i = 0; i < ARRAY_SIZE(timer_array); i++) {
        if (timer_id == timer_array[i].id) {
            timer_array[i].tot_exp += exp_num;
            LOGE("Timer id: %d, exp count: %d\n", timer_id, timer_array[i].tot_exp);
        }
    }
    LOGI("==============================================================\n");
}

int main(int argc, char* argv[]) {
    int error = 0;

    signal_handler = _new(struct signal_handler, signal_handler);
    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    timer_manager = get_timer_manager();

    error = timer_manager->init();
    if (error < 0) {
        LOGE("Failed to init timer manager\n");
        return -1;
    }

    if (!timer_manager->is_init()) {
        LOGE("Time manager has not init\n");
        return -1;
    }

    for (int i = 0; i < ARRAY_SIZE(timer_array); i++) {
        timer_array[i].id = timer_manager->alloc_timer(TIMER_INIT_EXP_MS,
                TIMER_INTERVAL_MS, TIMER_ONESHOT, timer_event_listener);
        if (timer_array[i].id < 0) {
            LOGE("Failed to alloc timer\n");
            return -1;
        }

        error = timer_manager->start(timer_array[i].id);
        if (error < 0) {
            LOGE("Failed to start timer(%d)\n", timer_array[i].id);
            return -1;
        }
    }

    LOGI("Waiting about %dms for timers start\n", TIMER_INIT_EXP_MS);
    while (1)
        sleep(1000);

    return 0;
}
