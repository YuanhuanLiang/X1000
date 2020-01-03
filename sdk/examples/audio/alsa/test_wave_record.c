#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <input/input_manager.h>
#include <audio/alsa/wave_recorder.h>
#include <audio/alsa/mixer_controller.h>
#include <thread/thread.h>

#define LOG_TAG "test_wave_record"

#define DEFAULT_CHANNELS         (2)
#define DEFAULT_SAMPLE_RATE      (8000)
#define DEFAULT_SAMPLE_LENGTH    (16)
#define DEFAULT_DURATION_TIME    (10)

enum {
    STATE_RECORDING,
    STATE_CANCEL,
};

/*
 * DMIC
 */
//const char* snd_device = "hw:0,2";

/*
 * AMIC
 */
const char* snd_device = "hw:0,0";
static struct wave_recorder* recorder;
static struct mixer_controller* mixer;
static struct input_manager* input_manager;
static struct thread* record_runner;
static struct signal_handler* signal_handler;
static int record_state = STATE_RECORDING;
static int fd;

static void handle_signal(int signal) {
    _delete(record_runner);

    if (input_manager) {
        input_manager->stop();
        input_manager->deinit();
    }

    if (recorder)
        recorder->deinit();
    if (mixer)
        mixer->deinit();

    if (fd > 0)
        close(fd);

    exit(1);
}

static void input_event_listener(const char* name, struct input_event* event) {
    int error = 0;

    if (event->code == KEY_POWER) {
        if (event->value == 1)
            return;

        if (record_state == STATE_RECORDING) {

            record_state = STATE_CANCEL;
            error = recorder->cancel_record();
            if (error < 0) {
                LOGE("Failed to cancel play\n");
                return;
            }

            record_runner->stop(record_runner);

        } else {
            error = record_runner->start(record_runner, NULL);
            if (error < 0) {
                LOGE("Failed to start play runner\n");
                return;
            }

            record_state = STATE_RECORDING;
        }

    }
}

static void record_thread(struct pthread_wrapper* thread, void* param) {
    int error = 0;

    error = recorder->record_wave(fd, DEFAULT_CHANNELS,
            DEFAULT_SAMPLE_RATE, DEFAULT_SAMPLE_LENGTH, DEFAULT_DURATION_TIME);
    if (error < 0)
        LOGE("Failed to record wave file\n");

    pthread_exit(NULL);
}

static void play_cleanup(void) {
    LOGI("Thread cleanup here\n");
}

int main(int argc, char *argv[]) {
    int error = 0;

    if (argc != 3) {
        LOGE("Usage: %s FILE.wav VOLUME\n", argv[0]);
        return -1;
    }

    remove(argv[1]);

    fd = open(argv[1], O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        LOGE("Failed to create %s\n", argv[1]);
        return -1;
    }

    signal_handler = _new(struct signal_handler, signal_handler);
    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    input_manager = get_input_manager();
    error = input_manager->init();
    if (error < 0) {
        LOGE("Failed to init input_manager\n");
        return -1;
    }

    input_manager->register_event_listener("gpio-keys", input_event_listener);
    input_manager->start();

    record_runner = _new(struct thread, thread);
    record_runner->runnable.run = record_thread;
    record_runner->runnable.cleanup = play_cleanup;

    recorder = get_wave_recorder();
    error = recorder->init(snd_device);
    if (error < 0) {
        LOGE("Failed to init recorder\n");
        return -1;
    }

    mixer = get_mixer_controller();
    error = mixer->init();
    if (error < 0) {
        LOGE("Failed to init mixer\n");
        goto error;
    }

    int volume = strtol(argv[2], NULL, 10);

    error = mixer->set_volume("Digital", CAPTURE, volume);
    if (error < 0) {
        LOGE("Failed to set volume\n");
        goto error;
    }

    volume = mixer->get_volume("Digital", CAPTURE);
    if (volume < 0) {
        LOGE("Failed to get_volume\n");
        goto error;
    }

    LOGI("Record Volume: %d\n", volume);

    record_runner->start(record_runner, NULL);

    while (1)
        sleep(1000);

    return 0;

error:
    if (fd > 0)
        close(fd);

    return -1;
}
