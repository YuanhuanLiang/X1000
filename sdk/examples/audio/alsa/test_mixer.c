#include <string.h>
#include <stdlib.h>

#include <types.h>
#include <utils/log.h>
#include <audio/alsa/mixer_controller.h>

#define LOG_TAG "test_mixer_controller"

static struct mixer_controller* mixer;

static void print_help(void) {
    LOGE("Usage: %s volume/mute VOLUME/MUTE\n", LOG_TAG);
}

int main(int argc, char *argv[]) {
    int error = 0;
    int playback_vol;
    int record_vol;

    if (argc != 3) {
        print_help();
        return -1;
    }

    mixer = get_mixer_controller();
    error = mixer->init();
    if (error < 0) {
        LOGE("Failed to init mixer\n");
        return -1;
    }

    if (!strcmp(argv[1], "volume")) {
        error = mixer->set_volume("MERCURY", PLAYBACK, strtol(argv[2], NULL, 10));
        if (error < 0) {
            LOGE("Failed to set playback volume\n");
            return -1;
        }

        error = mixer->set_volume("Digital", CAPTURE, strtol(argv[2], NULL, 10));
        if (error < 0) {
            LOGE("Failed to set record volume\n");
            return -1;
        }

        playback_vol = mixer->get_volume("MERCURY", PLAYBACK);
        record_vol = mixer->get_volume("MERCURY", CAPTURE);

        LOGI("playback volume: %d, record volume: %d\n", playback_vol,
                record_vol);

    } else if (!strcmp(argv[1], "mute")) {
        error = mixer->mute("Digital Playback mute", strtol(argv[2], NULL, 10));
        if (error < 0) {
            LOGE("Failed to mute\n");
            return -1;
        }

    } else {
        print_help();
    }
}
