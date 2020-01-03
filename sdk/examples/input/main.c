#include <unistd.h>

#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <input/input_manager.h>

#define LOG_TAG "test_input"

static struct input_manager *input_manager;
static struct signal_handler* signal_handler;

static void input_event_listener(const char* input_name,
        struct input_event* event) {
    LOGI("Input device name: %s\n", input_name);
    input_manager->dump_event(event);
}

static void handle_signal(int signal) {
    int error = 0;

    input_manager->unregister_event_listener("gpio-keys", input_event_listener);

    if (input_manager->is_start()) {
        error = input_manager->stop();
        if (error < 0)
            LOGE("Failed to stop input manager\n");
    }

    error = input_manager->deinit();
    if (error < 0)
        LOGE("Failed to deinit input manager\n");

    exit(1);
}

int main(int argc, char *argv[]) {
    int error = 0;

    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    input_manager = get_input_manager();

    error = input_manager->init();
    if (error < 0) {
        LOGE("Failed to init input manager\n");
        return -1;
    }

    error = input_manager->start();
    if (error < 0) {
        LOGE("Failed to start input manager\n");
        return -1;
    }

    input_manager->register_event_listener("gpio-keys", input_event_listener);

    while (1)
        sleep(1000);

    return 0;
}
