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

#ifndef LIB_SHA1_H
#define LIB_SHA1_H

/*
 * Deprecated, you should use openssl(include/lib/openssl)
 */
#define SHA1_DIGEST_LENGTH 20

typedef struct
{
    unsigned long h[5];
    unsigned long count_lo;
    unsigned long count_hi;
    unsigned char buf[64];

} SHA1_CTX;

void SHA1Transform(unsigned long state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1Final(SHA1_CTX* context, unsigned char digest[20]);
void SHA1(unsigned char* buf, unsigned int size, unsigned char result[20]);

#endif /* LIB_SHA1_H */
