#include <unistd.h>
#include <stdlib.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <alarm/alarm_manager.h>

#define LOG_TAG "test_alarm"


static struct alarm_manager* alarm_manager;
static struct signal_handler* signal_handler;

static void alarm_listener1(void) {
    LOGI("=====> %s ring <=====\n", __FUNCTION__);
}

static void alarm_listener2(void) {
    LOGI("=====> %s ring <=====\n", __FUNCTION__);
}

static void alarm_listener3(void) {
    LOGI("=====> %s ring <=====\n", __FUNCTION__);
}

static void handle_signal(int signal) {
    alarm_manager->stop();
    alarm_manager->deinit();

    exit(1);
}

int main(int argc, char *argv[]) {
    alarm_manager = get_alarm_manager();

    signal_handler = _new(struct signal_handler, signal_handler);
    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    alarm_manager->init();
    alarm_manager->start();

    uint64_t cur_timems = alarm_manager->get_sys_time_ms();

    alarm_manager->set(cur_timems + 5 * 1000, alarm_listener1);
    alarm_manager->set(cur_timems + 15 * 1000, alarm_listener2);
    alarm_manager->set(cur_timems + 10 * 1000, alarm_listener3);

    sleep(1);

    alarm_manager->cancel(alarm_listener1);

    while(1)
        sleep(1000);

    return 0;
}
