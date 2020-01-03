/*
 *  Copyright (C) 2016, Zhang YanMing <jamincheung@126.com>
 *
 *  Linux recovery updater
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

#ifndef LIB_CRC_H
#define LIB_CRC_H

#include <stdint.h>

/**
 * crc7 - update the CRC7 for the data buffer
 * @crc:     previous CRC7 value
 * @buffer:  data pointer
 * @len:     number of bytes in the buffer
 * Context: any
 *
 * Returns the updated CRC7 value.
 */
uint8_t local_crc7(uint8_t crc, const uint8_t *buffer, size_t len);

/**
 * crc8() - Calculate and return CRC-8 of the data
 *
 * This uses an x^8 + x^2 + x + 1 polynomial.  A table-based algorithm would
 * be faster, but for only a few bytes it isn't worth the code size
 *
 * @crc_start: CRC8 start value
 * @vptr: Buffer to checksum
 * @len: Length of buffer in bytes
 * @return CRC8 checksum
 */
uint32_t local_crc8(uint32_t crc_start, const uint8_t *vptr, int32_t len);

uint16_t local_crc16(uint8_t *s, uint32_t len);

uint32_t local_crc32(uint32_t val, const void *ss, int32_t len);

#endif /* LIB_CRC_H */
