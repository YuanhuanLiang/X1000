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

#ifndef WAVE_PARSER_H
#define WAVE_PAESER_H

#include <types.h>
#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d) ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)             (v)
#define LE_INT(v)               (v)
#define BE_SHORT(v)             bswap_16(v)
#define BE_INT(v)               bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d) ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)             bswap_16(v)
#define LE_INT(v)               bswap_32(v)
#define BE_SHORT(v)             (v)
#define BE_INT(v)               (v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF        COMPOSE_ID('R','I','F','F')
#define WAV_WAVE        COMPOSE_ID('W','A','V','E')
#define WAV_FMT         COMPOSE_ID('f','m','t',' ')
#define WAV_DATA        COMPOSE_ID('d','a','t','a')

/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092
#define WAV_FMT_EXTENSIBLE      0xfffe

/* Used with WAV_FMT_EXTENSIBLE format */
#define WAV_GUID_TAG        "/x00/x00/x00/x00/x10/x00/x80/x00/x00/xAA/x00/x38/x9B/x71"

/* it's in chunks like .voc and AMIGA iff, but my source say there
   are in only in this combination, so I combined them in one header;
   it works on all WAVE-file I have
 */
typedef struct {
    uint32_t magic;        /* 'RIFF' */
    uint32_t length;       /* filelen */
    uint32_t type;         /* 'WAVE' */
} WaveHeader;

typedef struct {
    uint32_t magic;
    uint32_t format_size;
    uint16_t format;        /* see WAV_FMT_* */
    uint16_t channels;
    uint32_t sample_fq;     /* frequence of sample */
    uint32_t byte_p_sec;
    uint16_t byte_p_spl;    /* samplesize; 1 or 2 bytes */
    uint16_t bit_p_spl;     /* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
    WaveFmtBody format;
    uint16_t ext_size;
    uint16_t bit_p_spl;
    uint32_t channel_mask;
    uint16_t guid_format;    /* WAV_FMT_* */
    uint8_t guid_tag[14];    /* WAV_GUID_TAG */
} WaveFmtExtensibleBody;

typedef struct {
    uint32_t type;         /* 'data' */
    uint32_t length;       /* samplecount */
} WaveChunkHeader;

typedef struct {
    WaveHeader header;
    WaveFmtBody format;
    WaveChunkHeader chunk_header;
} WaveContainer;

int wave_read_header(int fd, WaveContainer* container);
int wave_write_header(int fd, WaveContainer* container);


#endif /* WAVE_PARSER_H */
