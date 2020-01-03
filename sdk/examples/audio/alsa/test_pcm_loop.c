#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <signal/signal_handler.h>
#include <audio/alsa/wave_player.h>
#include <audio/alsa/wave_recorder.h>
#include <audio/alsa/mixer_controller.h>

#define LOG_TAG "test_pcm_loop"

#define DEFAULT_CHANNELS         (2)
#define DEFAULT_SAMPLE_RATE      (441000)
#define DEFAULT_SAMPLE_LENGTH    (16)

const char* play_device = "hw:0,0";
const char* record_device = "hw:0,0";
static struct wave_recorder* recorder;
static struct wave_player* player;
static struct mixer_controller* mixer;
static struct signal_handler* signal_handler;

static void handle_signal(int signal) {
    if (player)
        player->deinit();

    if (recorder)
        recorder->deinit();

    if (mixer)
        mixer->deinit();

    exit(1);
}

int main(int argc, char *argv[]) {
    int error = 0;
    int buffer_size;
    uint8_t* buffer;

    if (argc != 3) {
        LOGE("Usage: %s REC-VOLUME PLAY-VOLUME\n", argv[0]);
        return -1;
    }

    signal_handler = _new(struct signal_handler, signal_handler);
    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    recorder = get_wave_recorder();
    error = recorder->init(record_device);
    if (error < 0) {
        LOGE("Failed to init recorder\n");
        return -1;
    }

    player = get_wave_player();
    error = player->init(play_device);
    if (error < 0) {
        LOGE("Failed to init player\n");
        return -1;
    }

    mixer = get_mixer_controller();
    error = mixer->init();
    if (error < 0) {
        LOGE("Failed to init mixer\n");
        return -1;
    }

    int rec_volume = strtol(argv[1], NULL, 10);
    int play_volume = strtol(argv[2], NULL, 10);

    error = mixer->set_volume("Digital", CAPTURE, rec_volume);
    if (error < 0) {
        LOGE("Failed to set volume\n");
        return -1;
    }

    rec_volume = mixer->get_volume("Digital", CAPTURE);
    if (rec_volume < 0) {
        LOGE("Failed to get_volume\n");
        return -1;
    }

    LOGI("Record Volume: %d\n", rec_volume);

    error = mixer->set_volume("MERCURY", PLAYBACK, play_volume);
    if (error < 0) {
        LOGE("Failed to set volume\n");
        return -1;
    }

    play_volume = mixer->get_volume("MERCURY", PLAYBACK);
    if (play_volume < 0) {
        LOGE("Failed to get_volume\n");
        return -1;
    }

    LOGI("Playback Volume: %d\n", play_volume);

    for (;;) {
        buffer_size = recorder->record_stream(DEFAULT_CHANNELS, DEFAULT_SAMPLE_RATE,
                DEFAULT_SAMPLE_LENGTH, &buffer);
        if (buffer_size < 0) {
            LOGE("Failed to record stream\n");
            return -1;
        }

        error = player->play_stream(DEFAULT_CHANNELS, DEFAULT_SAMPLE_RATE,
                DEFAULT_SAMPLE_LENGTH, buffer, buffer_size);
        if (error < 0) {
            LOGE("Failed to playback stream\n");
            return -1;
        }
    }

    return 0;
}
