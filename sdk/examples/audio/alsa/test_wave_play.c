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
#include <audio/alsa/wave_player.h>
#include <audio/alsa/mixer_controller.h>
#include <thread/thread.h>

#define LOG_TAG "test_wave_play"

//#define KEY_START_STOP

enum {
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_CANCEL,
};

const char* snd_device = "default";
static struct wave_player* player;
static struct mixer_controller* mixer;
static struct input_manager* input_manager;
static struct thread* play_runner;
static struct signal_handler* signal_handler;
static int play_state = STATE_PLAYING;
static int fd;

static void handle_signal(int signal) {
    _delete(play_runner);

    if (input_manager) {
        input_manager->stop();
        input_manager->deinit();
    }

    if (player)
        player->deinit();
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

#ifdef KEY_START_STOP
        if (play_state == STATE_PLAYING) {

            play_state = STATE_CANCEL;
            error = player->cancel_play();
            if (error < 0) {
                LOGE("Failed to cancel play\n");
                return;
            }

            play_runner->stop(play_runner);

        } else {
            error = play_runner->start(play_runner, NULL);
            if (error < 0) {
                LOGE("Failed to start play runner\n");
                return;
            }

            play_state = STATE_PLAYING;
        }

#else
        if (play_state == STATE_PLAYING) {
            error = player->pause_play();
            if (error < 0) {
                LOGE("Failed to pause play\n");
                return;
            }
            play_state = STATE_PAUSE;

        } else if (play_state == STATE_PAUSE){
            error = player->resume_play();
            if (error < 0) {
                LOGE("Failed to resume play\n");
                return;
            }
            play_state = STATE_PLAYING;
        }
#endif
    }
}

static void play_thread(struct pthread_wrapper* thread, void* param) {
    int error = 0;

    error = player->play_wave(fd);
    if (error < 0)
        LOGE("Failed to play wave\n");

    if (fd > 0)
        close(fd);

    fd = -1;

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

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        LOGE("Failed to open %s\n", argv[1]);
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

    play_runner = _new(struct thread, thread);
    play_runner->runnable.run = play_thread;
    play_runner->runnable.cleanup = play_cleanup;

    player = get_wave_player();
    error = player->init(snd_device);
    if (error < 0) {
        LOGE("Failed to init player\n");
        goto error;
    }

    mixer = get_mixer_controller();
    error = mixer->init();
    if (error < 0) {
        LOGE("Failed to init mixer\n");
        goto error;
    }

    int volume = strtol(argv[2], NULL, 10);

    error = mixer->set_volume("MERCURY", PLAYBACK, volume);
    if (error < 0) {
        LOGE("Failed to set volume\n");
        goto error;
    }

    volume = mixer->get_volume("MERCURY", PLAYBACK);
    if (volume < 0) {
        LOGE("Failed to get_volume\n");
        goto error;
    }

    LOGI("Playback Volume: %d\n", volume);

    play_runner->start(play_runner, NULL);
    while (1)
        sleep(1000);

    return 0;

error:
    if (fd > 0)
        close(fd);
    return -1;
}
