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

#ifndef WAVE_RECODER_H
#define WAVE_RECODER_H

struct wave_recorder {
    int (*init)(const char* snd_device);
    int (*deinit)(void);
    int (*record_wave)(int fd, int channels, int sample_rate, int sample_length,
            int duration_time);
    int (*record_stream)(int channels, int sample_rate, int sample_length,
            uint8_t** buffer);
    int (*cancel_record)(void);
};

struct wave_recorder* get_wave_recorder(void);

#endif /* WAVE_RECODER_H */
