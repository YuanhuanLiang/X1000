/*
 * drivers/misc/lock_cylinder.c
 *
 * Smartdoor lock cylinder driver.
 * This driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com / panddio@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/signal.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/lock_cylinder.h>
#include <asm/atomic.h>
#include <crypto/sha.h>

/*
 * IOCTL commands
 */
#define LOCK_CYLINDER_IOC_MAGIC                 'L'
#define LOCK_CYLINDER_IOC_SET_SECRET            _IOW(LOCK_CYLINDER_IOC_MAGIC, 0, int)
#define LOCK_CYLINDER_IOC_R_KEY_ROMID           _IOR(LOCK_CYLINDER_IOC_MAGIC, 1, int)
#define LOCK_CYLINDER_IOC_G_KEY_STATUE          _IOR(LOCK_CYLINDER_IOC_MAGIC, 2, int)
#define LOCK_CYLINDER_IOC_G_KEY_STATUE_BLOCK    _IOR(LOCK_CYLINDER_IOC_MAGIC, 3, int)
#define LOCK_CYLINDER_IOC_REGISTER_KEY          _IOW(LOCK_CYLINDER_IOC_MAGIC, 4, int)
#define LOCK_CYLINDER_IOC_AUTHENTICATE_KEY      _IOW(LOCK_CYLINDER_IOC_MAGIC, 5, int)
#define LOCK_CYLINDER_IOC_PWR_DOWN              _IOW(LOCK_CYLINDER_IOC_MAGIC, 6, int)
#define LOCK_CYLINDER_IOC_PWR_ENABLE            _IOW(LOCK_CYLINDER_IOC_MAGIC, 7, int)


/**
 * DS28Exx ROM function commands
 */
#define READ_ROM     0x33
#define MATCH_ROM    0x55
#define SKIP_ROM     0xCC

#define W_MEMORY_FUNCTION_CMD                0x55
#define R_MEMORY_FUNCTION_CMD                0xF0
#define W_BLOCK_PROTECTION_CMD               0xC3
#define AUTHENTICATE_W_BLCOK_PROTECITON_CMD  0xCC
#define R_PROTECTION_CMD                     0x80
#define W_PROTECTION_CMD                     0x40
#define EEPROM_PROTECTION_CMD                0x20
#define AUTHENTICATION_CMD                   0x10
#define R_STATUS_CMD                         0xAA
#define RW_SCRATCHPAD_CMD                    0x0F
#define LOAD_AND_LOCK_SECRET_CMD             0x33
#define COMPUTE_AND_LOCK_SECRET_CMD          0x3C
#define COMPUTE_AND_READ_PAGEMCA_CMD         0xA5
#define AUTHENTICATE_W_MEMORY_CMD            0x5A


#define LOOP_NUMBER    5

//typedef unsigned char u8;
//typedef unsigned short u16;
//typedef unsigned long u32;

enum retval_t {
    NO_DEVICE,
    READ_OK,
    READ_FAILED,
    CRC_ERROR,
    UNMATCH_MAC,
    MATCH_MAC,
    WRITE_OK,
    WRITE_FAILED,
    EXISTED_KEY,
    NOEXIST_KEY,
    DELETE_OK,
};

enum page_t {
    PAGE0,
    PAGE1,
};

enum segment_t {
    SEGMENT0,
    SEGMENT1,
};

/**
 * Driver data struct
 */
struct lock_cylinder_drvdata {
    int detect_irq;

    u8 romid[8];
    u8 secret[32];
    u8 rd_status[4];
    u8 mac_value[32];
    u8 mt_digest[120];

    bool is_set_secret;
    bool is_wait_passed;
    bool is_key_inserted;
    bool is_key_certified;

    atomic_t opened;
    wait_queue_head_t wait;
    struct device *dev;
    struct mutex lock;
    struct timer_list timer;
    struct work_struct work;
    struct miscdevice miscdev;
    struct lock_cylinder_platform_data *pdata;
};

static u32 sha_constants[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    0xca273ece, 0xd186b8c7, 0xeada7dd6, 0xf57d4f7f, 0x06f067aa, 0x0a637dc5, 0x113f9804, 0x1b710b35,
    0x28db77f5, 0x32caab7b, 0x3c9ebe0a, 0x431d67c4, 0x4cc5d4be, 0x597f299c, 0x5fcb6fab, 0x6c44198c
};

static u32 H32[8];
static u32 W32[16];
static u32 a32, b32, c32, d32, e32, f32, g32, h32;

static const u8 crc8_table[] = {
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

static u8 docrc8(u8 x, u8 crc8)
{
    return crc8_table[crc8 ^ x];
}

static u16 docrc16(u16 x, u16 crc16)
{
    u16 oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    x = (x ^(crc16 & 0xff)) & 0xff;
    crc16 >>= 8;

    if (oddparity[x & 0xf] ^ oddparity[x >> 4])
        crc16 ^= 0xc001;

    x <<= 6;
    crc16 ^= x;
    x <<= 1;
    crc16 ^= x;

    return crc16;
}

/**
 * SHA-256 support function
 */
static inline u32 sha_ch(u32 x, u32 y, u32 z)
{
    return (x & y) ^ ((~x) & z);
}

static inline u32 sha_maj(u32 x, u32 y, u32 z)
{
    u32 temp = x & y;
    temp ^= (x & z);
    temp ^= (y & z);
    return temp;
}

static inline u32 sha_rotr_32(u32 val, u16 r)
{
    val = val & 0xFFFFFFFFUL;
    return ((val >> r) | (val << (32 - r))) & 0xFFFFFFFFUL;
}

static inline u32 sha_shr_32(u32 val, u16 r)
{
    val = val & 0xFFFFFFFFUL;
    return val >> r;
}

static inline u32 sha_bigsigma256_0(u32 x)
{
    return sha_rotr_32(x,2) ^ sha_rotr_32(x,13) ^ sha_rotr_32(x,22);
}

static inline u32 sha_bigsigma256_1(u32 x)
{
    return sha_rotr_32(x,6) ^ sha_rotr_32(x,11) ^ sha_rotr_32(x,25);
}

static inline u32 sha_littlesigma256_0(u32 x)
{
    return sha_rotr_32(x,7) ^ sha_rotr_32(x,18) ^ sha_shr_32(x,3);
}

static inline u32 sha_littlesigma256_1(u32 x)
{
    return sha_rotr_32(x,17) ^ sha_rotr_32(x,19) ^ sha_shr_32(x,10);
}

static void sha_copyWordsToBytes32(u32* input, u8* output, u16 numwords)
{
    u16 i;
    u32 temp;

    for (i = 0; i < numwords; i++) {
        temp = *input++;
        *output++ = (u8)(temp >> 24);
        *output++ = (u8)(temp >> 16);
        *output++ = (u8)(temp >> 8);
        *output++ = (u8)(temp);
    }
}

static void sha_writeResult(u16 reverse, u8* outpointer)
{
    u8 i, tmp;

    sha_copyWordsToBytes32(H32, outpointer, 8);
    if (reverse) {
        for (i = 0; i < 16; i++) {
            tmp = outpointer[i];
            outpointer[i] = outpointer[31-i];
            outpointer[31-i] = tmp;
        }
    }
}

static u32 sha_getW(int index)
{
    u32 newW;

    if (index < 16) {
        return W32[index];
    }

    newW = sha_littlesigma256_1(W32[(index-2)&0x0f]) + W32[(index-7)&0x0f] +
           sha_littlesigma256_0(W32[(index-15)&0x0f]) + W32[(index-16)&0x0f];

    W32[index & 0x0f] = newW & 0xFFFFFFFFUL;

    return newW;
}

static void sha_prepareSchedule(u8* message)
{
    u8 i, j;
    u32 temp;

    for (i = 0; i < 16; i++) {
        temp = 0;
        for (j = 0; j < 4; j++) {
            temp = temp << 8;
            temp = temp | (*message & 0xff);
            message++;
        }
        W32[i] = temp;
    }
}

static void sha256_hashblock(u8* message, u16 lastblock)
{
    u16 i;
    u16 sha1counter = 0;
    u16 sha1functionselect = 0;
    u32 nodeT1, nodeT2;
    u32 Wt, Kt;

    sha_prepareSchedule(message);

    a32 = H32[0];
    b32 = H32[1];
    c32 = H32[2];
    d32 = H32[3];
    e32 = H32[4];
    f32 = H32[5];
    g32 = H32[6];
    h32 = H32[7];

    for (i = 0; i < 64; i++) {
        Wt = sha_getW(i);
        Kt = sha_constants[i];

        nodeT1 = (h32 + sha_bigsigma256_1(e32) + sha_ch(e32,f32,g32) + Kt + Wt);
        nodeT2 = (sha_bigsigma256_0(a32) + sha_maj(a32,b32,c32));
        h32 = g32;
        g32 = f32;
        f32 = e32;
        e32 = d32 + nodeT1;
        d32 = c32;
        c32 = b32;
        b32 = a32;
        a32 = nodeT1 + nodeT2;

        sha1counter++;
        if (sha1counter == 20) {
            sha1functionselect++;
            sha1counter = 0;
        }
    }

    if (!lastblock) {
        H32[0] += a32;
        H32[1] += b32;
        H32[2] += c32;
        H32[3] += d32;
        H32[4] += e32;
        H32[5] += f32;
        H32[6] += g32;
        H32[7] += h32;
    } else {
        H32[0] = a32;
        H32[1] = b32;
        H32[2] = c32;
        H32[3] = d32;
        H32[4] = e32;
        H32[5] = f32;
        H32[6] = g32;
        H32[7] = h32;
    }
}

static void compute_sha256(struct lock_cylinder_drvdata *ilock,
                u16 length, u16 skipconst, bool reverse, u8 *digest)
{
#define WORDSIZE  (32) //wordsize is 32 bits
    u16 i, j;
    u16 bytes_per_block;
    u16 nonpaddedlength;
    u16 numblocks;
    u16 markerwritten;
    u16 lastblock;
    u32 bitlength;
    u8 workbuffer[120];
    u8 *message = ilock->mt_digest;

    bytes_per_block = 16 * (WORDSIZE / 8);
    nonpaddedlength = length + 1 + (WORDSIZE / 4);
    numblocks = nonpaddedlength / bytes_per_block;
    if (nonpaddedlength % bytes_per_block != 0) {
        numblocks++;
    }

    H32[0] = SHA256_H0;
    H32[1] = SHA256_H1;
    H32[2] = SHA256_H2;
    H32[3] = SHA256_H3;
    H32[4] = SHA256_H4;
    H32[5] = SHA256_H5;
    H32[6] = SHA256_H6;
    H32[7] = SHA256_H7;

    bitlength = 8 * length;
    markerwritten = 0;

    for (i = 0; i < numblocks; i++) {
        if (length > bytes_per_block) {
            memcpy(workbuffer, message, bytes_per_block);
            length -= bytes_per_block;
        } else if (length == bytes_per_block) {
            memcpy(workbuffer, message, length);
            length = 0;
        } else {
            memcpy(workbuffer, message, length);
            message = workbuffer + length;

            if (markerwritten == 0) {
                *message++ = 0x80;
                length++;
            }

            while(length < bytes_per_block) {
                *message++ = 0;
                length++;
            }

            length = 0;
            markerwritten = 1;
        }

        /**
         * On the last block, put the bit length at the very end
         */
        lastblock = (i == (numblocks - 1));
        if (lastblock) {
            message = workbuffer + bytes_per_block - 1;
            for (j = 0; j < WORDSIZE / 4; j++) {
                *message-- = (u8)bitlength;
                bitlength = bitlength >> 8;
            }
        }

        /**
         * SHA in software
         */
        sha256_hashblock(workbuffer, (u16)(lastblock && skipconst));
        message += bytes_per_block;
    }

    sha_writeResult(reverse, digest);
}

static void ds28exx_compute_mac256(struct lock_cylinder_drvdata *ilock, u16 length, u8 *mac)
{
    if (length == 119) {
        /* Insert secret */
        memcpy(&ilock->mt_digest[64], ilock->secret, 32);

        /**
         * Change to little endian for A1 devices
         */
        if (ilock->pdata->reverse_endian) {
            u8 i, j;
            u8 temp[4];

            for (i = 0; i < 108; i += 4) {
                for (j = 0; j < 4; j++)
                    temp[3-j] = ilock->mt_digest[i+j];
                for (j = 0; j < 4; j++)
                    ilock->mt_digest[i+j] = temp[j];
            }
        }

        compute_sha256(ilock, 119, 1, (bool)1, mac);
    }
}

static u8 ds28exx_verify_mac256(struct lock_cylinder_drvdata *ilock, u16 length)
{
    u8 i, calc_mac[32];

    /* Calculate the MAC */
    ds28exx_compute_mac256(ilock, length, calc_mac);

    /**
     * Compare calculated mac with one read from device
     */
    for (i = 0; i < 32; i++) {
        if (ilock->mac_value[i] != calc_mac[i]) {
            return UNMATCH_MAC;
        }
    }

    return MATCH_MAC;
}

static u8 ds28exx_reset(struct lock_cylinder_platform_data *pdata)
{
    u8 retval;
    unsigned long flags;

    local_irq_save(flags);
    gpio_direction_output(pdata->id_pin, 1);
    udelay(10);

    gpio_set_value(pdata->id_pin, 0);
    udelay(60);
    gpio_set_value(pdata->id_pin, 1);

    gpio_direction_input(pdata->id_pin);
    udelay(10);
    retval = gpio_get_value(pdata->id_pin);

    gpio_direction_output(pdata->id_pin, 1);
    local_irq_restore(flags);
    udelay(30);

    return retval;
}

static u8 ds28exx_read_byte(struct lock_cylinder_platform_data *pdata)
{
    u8 i, retval = 0, recvbit = 0;
    unsigned long flags;

    local_irq_save(flags);
    gpio_direction_output(pdata->id_pin, 1);
    for (i = 0; i < 8; i++) {
        gpio_set_value(pdata->id_pin, 0);
        udelay(1);
        gpio_set_value(pdata->id_pin, 1);

        gpio_direction_input(pdata->id_pin);
        recvbit = gpio_get_value(pdata->id_pin);

        gpio_direction_output(pdata->id_pin, 1);
        udelay(5);

        retval = (recvbit << i) | retval;
    }
    local_irq_restore(flags);
    return retval;
}

static void ds28exx_write_byte(struct lock_cylinder_platform_data *pdata, u8 value)
{
    u8 i;
    unsigned long flags;

    local_irq_save(flags);
    gpio_direction_output(pdata->id_pin, 1);
    for (i = 0; i < 8; i++) {
        if (value & 0x01) {
            gpio_set_value(pdata->id_pin, 0);
            udelay(2);
            gpio_set_value(pdata->id_pin, 1);
            udelay(8);
        } else {
            gpio_set_value(pdata->id_pin, 0);
            udelay(8);
            gpio_set_value(pdata->id_pin, 1);
            udelay(2);
        }
        value >>= 1;
    }
    local_irq_restore(flags);
}

static char ds28exx_read_romid(struct lock_cylinder_platform_data *pdata, u8 *pbuf)
{
    u8 i, crc8;
    u8 temp = 0, loop = 0;

again:
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, READ_ROM);
    udelay(10);

    for (i = 0; i < 8; i++) {
        pbuf[i] = ds28exx_read_byte(pdata);
        temp += pbuf[i];
    }

    if (temp <= 0)
        return READ_FAILED;

    for (i = 0, crc8 = 0; i < 8; i++)
        crc8 = docrc8(pbuf[i], crc8);

    if (crc8 != 0) {
        if (loop < LOOP_NUMBER) {
            temp = 0;
            goto again;
        }
        return CRC_ERROR;
    }

    return READ_OK;
}

static u8 ds28exx_read_block_status(struct lock_cylinder_platform_data *pdata,
            u8 blocknum, bool personalitybyte, u8 *pstatbuf)
{
    u8 temp[40];
    u8 i, cnt;
    u8 loop = 0;
    u16 crc16 = 0;

again:
    cnt = 0;
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, SKIP_ROM);

    /**
     * Construct a packet to send
     */
    temp[cnt++] = R_STATUS_CMD;
    temp[cnt++] = personalitybyte ? 0xE0 : (blocknum & 0x03);
    for (i = 0; i < cnt; i++)
        ds28exx_write_byte(pdata, temp[i]);

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    if (personalitybyte) {
        for (i = 0, cnt = 0; i < 4; i++, cnt++) {
            pstatbuf[i] = ds28exx_read_byte(pdata);
            temp[cnt] = pstatbuf[i];
        }
    } else {
        for (i = blocknum, cnt = 0; i < 4; i++, cnt++) {
            pstatbuf[cnt] = ds28exx_read_byte(pdata);
            temp[cnt] = pstatbuf[cnt];
        }
    }

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    return 0xAA;
}

static u8 ds28exx_read_user_memory(struct lock_cylinder_platform_data *pdata,
            u8 segment, u8 page, u8 *precvbuf)
{
    u8 temp[40];
    u8 i, cnt;
    u8 loop = 0;
    u16 crc16 = 0;

again:
    cnt = 0;
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, SKIP_ROM);

    /**
     * Construct a packet to send
     */
    temp[cnt++] = R_MEMORY_FUNCTION_CMD;
    temp[cnt++] = (segment << 5 | page) & 0xE1;
    for (i = 0; i < cnt; i++)
        ds28exx_write_byte(pdata, temp[i]);

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    /**
     * Receive EEPROM data
     */
    for (i = segment * 4, cnt = 0; i < 32; i++, cnt++) {
        precvbuf[cnt] = ds28exx_read_byte(pdata);
        temp[cnt] = precvbuf[cnt];
    }

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    return 0xAA;
}

static u8 ds28exx_compute_and_read_pagemac(struct lock_cylinder_platform_data *pdata,
            bool anonymous_mode, u8 pagenum, u8 *pmacbuf)
{
    u8 i, cnt, retval;
    u8 temp[40];
    u8 loop = 0;
    u16 crc16 = 0;

again:
    cnt = 0;
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, SKIP_ROM);

    /**
     * Construct a packet to send
     */
    temp[cnt++] = COMPUTE_AND_READ_PAGEMCA_CMD;
    temp[cnt++] = anonymous_mode ? (pagenum | 0xE0) : pagenum;
    for (i = 0; i < cnt; i++)
        ds28exx_write_byte(pdata, temp[i]);

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    udelay(6000); // Wait for sha256 calculation

    retval = ds28exx_read_byte(pdata);

    for (i = 0, cnt = 0; i < 32; i++, cnt++) {
        pmacbuf[i] = ds28exx_read_byte(pdata);
        temp[cnt] = pmacbuf[i];
    }

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    return retval;
}

static u8 ds28exx_readwrite_scratchpad(struct lock_cylinder_platform_data *pdata, bool readmode, u8 *pbuf)
{
    u8 temp[40];
    u8 i, cnt;
    u8 loop = 0;
    u16 crc16 = 0;

again:
    cnt = 0;
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, SKIP_ROM);

    /**
     * Construct a packet to send
     */
    temp[cnt++] = RW_SCRATCHPAD_CMD;
    temp[cnt++] = readmode ? 0x0F : 0x00;
    for (i = 0; i < cnt; i++)
        ds28exx_write_byte(pdata, temp[i]);

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    if (readmode) {
        for (i = 0, cnt = 0; i < 32; i++, cnt++) {
            pbuf[i] = ds28exx_read_byte(pdata);
            temp[cnt] = pbuf[i];
        }
    } else {
        for (i = 0, cnt = 0; i < 32; i++, cnt++) {
            ds28exx_write_byte(pdata, pbuf[i]);
            temp[cnt] = pbuf[i];
        }
    }

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    return 0xAA;
}

static u8 ds28exx_load_and_lock_secret(struct lock_cylinder_platform_data *pdata, bool lockenable)
{
    u8 i, cnt, retval;
    u8 temp[40];
    u8 loop = 0;
    u16 crc16 = 0;

again:
    cnt = 0;
    loop++;

    if (ds28exx_reset(pdata) != 0)
        return NO_DEVICE;

    ds28exx_write_byte(pdata, SKIP_ROM);

    /**
     * Construct a packet to send
     */
    temp[cnt++] = LOAD_AND_LOCK_SECRET_CMD;
    temp[cnt++] = lockenable ? 0xE0 : 0x00;
    for (i = 0; i < cnt; i++)
        ds28exx_write_byte(pdata, temp[i]);

    /**
     * Receive CRC
     */
    temp[cnt++] = ds28exx_read_byte(pdata);
    temp[cnt++] = ds28exx_read_byte(pdata);

    /**
     * Calculate CRC over this part
     */
    for (i = 0, crc16 = 0; i < cnt; i++)
        crc16 = docrc16(temp[i], crc16);

    if (crc16 != 0xB001) {
        if (loop < LOOP_NUMBER)
            goto again;
        return CRC_ERROR;
    }

    /**
     * Send release byte
     */
    ds28exx_write_byte(pdata, 0xAA);

    mdelay(50); /* wait for secret loading */

    retval = ds28exx_read_byte(pdata);
    if (retval != 0xAA && loop < LOOP_NUMBER) {
        goto again;
    }

    return retval;
}

static u8 ds28exx_set_and_lock_secret(struct lock_cylinder_platform_data *pdata, u8 *secretbuf, bool lockenable)
{
    u8 retval;

    retval = ds28exx_readwrite_scratchpad(pdata, (bool)0, secretbuf);
    if (retval != 0xAA)
        return retval;

    return ds28exx_load_and_lock_secret(pdata, lockenable);
}

static u8 ds28exx_authenticate_by_secret(struct lock_cylinder_drvdata *ilock)
{
    u8 retval;
    u8 scratchbuf[32];

#if 0
    memset(scratchbuf, 0, sizeof(scratchbuf));
#else
    get_random_bytes(scratchbuf, sizeof(scratchbuf));
#endif

    retval = ds28exx_read_romid(ilock->pdata, ilock->romid);
    if (retval != READ_OK)
        return UNMATCH_MAC;

    retval = ds28exx_read_block_status(ilock->pdata, 0, (bool)1, ilock->rd_status);
    if (retval != 0xAA)
        return UNMATCH_MAC;

    retval = ds28exx_readwrite_scratchpad(ilock->pdata, (bool)0, scratchbuf);
    if (retval != 0xAA)
        return UNMATCH_MAC;

    retval =  ds28exx_compute_and_read_pagemac(ilock->pdata, (bool)0, PAGE0, ilock->mac_value);
    if (retval != 0xAA)
        return UNMATCH_MAC;

    retval = ds28exx_read_user_memory(ilock->pdata, SEGMENT0, PAGE0, ilock->mt_digest);
    if (retval != 0xAA)
        return UNMATCH_MAC;

    memcpy(&ilock->mt_digest[32], scratchbuf, sizeof(scratchbuf));
    memcpy(&ilock->mt_digest[96], ilock->romid, sizeof(ilock->romid));
    memcpy(&ilock->mt_digest[104], &ilock->rd_status[2], 2);
    ilock->mt_digest[106] = 0x00;

    return ds28exx_verify_mac256(ilock, 119);
}

static int lock_cylinder_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct lock_cylinder_drvdata *ilock =
                container_of(miscdev, struct lock_cylinder_drvdata, miscdev);

    if (atomic_read(&ilock->opened)) {
        dev_err(ilock->dev, "busy, multi open is not supported\n");
        return -EBUSY;
    }

    atomic_inc(&ilock->opened);
    return 0;
}

static int lock_cylinder_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct lock_cylinder_drvdata *ilock =
                container_of(miscdev, struct lock_cylinder_drvdata, miscdev);

    atomic_dec(&ilock->opened);
    return 0;
}

static long lock_cylinder_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct lock_cylinder_drvdata *ilock =
                container_of(miscdev, struct lock_cylinder_drvdata, miscdev);
    void __user *argp = (void __user *)arg;

    switch(cmd) {
    case LOCK_CYLINDER_IOC_SET_SECRET:
        mutex_lock(&ilock->lock);
        if (!ilock->is_set_secret) {
            if (copy_from_user((void *)ilock->secret, argp,
                        sizeof(ilock->secret))) {
                goto err_ioctl_exit;
            }
            ilock->is_set_secret = true;
        }
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_R_KEY_ROMID:
        mutex_lock(&ilock->lock);
        if (ds28exx_read_romid(ilock->pdata, ilock->romid) == READ_OK) {
            if (copy_to_user(argp, (void *)ilock->romid,
                        sizeof(ilock->romid))) {
                goto err_ioctl_exit;
            }
        }
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_G_KEY_STATUE_BLOCK:
        if(filp->f_flags & O_NONBLOCK) {
            dev_err(ilock->dev, "IOCTL: error, dev is opened with O_NONBLOCK\n");
            return -EAGAIN;
        }
        if (wait_event_interruptible(ilock->wait, ilock->is_wait_passed)) {
            return -ERESTARTSYS;
        }
    case LOCK_CYLINDER_IOC_G_KEY_STATUE:
        mutex_lock(&ilock->lock);
        if (copy_to_user(argp, (void *)&ilock->is_key_inserted,
                    sizeof(ilock->is_key_inserted))) {
            goto err_ioctl_exit;
        }
        if (cmd == LOCK_CYLINDER_IOC_G_KEY_STATUE_BLOCK) {
            ilock->is_wait_passed = false;
        }
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_REGISTER_KEY:
        mutex_lock(&ilock->lock);
        if (ds28exx_read_romid(ilock->pdata, ilock->romid) != READ_OK) {
            dev_err(ilock->dev, "IOCTL: failed to read the key ROMID\n");
            goto err_ioctl_exit;
        }
        if (ds28exx_set_and_lock_secret(ilock->pdata, ilock->secret, (bool)0) != 0xAA) {
            dev_err(ilock->dev, "IOCTL: failed to set and lock secret\n");
            goto err_ioctl_exit;
        }
        if (ds28exx_authenticate_by_secret(ilock) != MATCH_MAC) {
            dev_err(ilock->dev, "IOCTL: failed to authenticate the key\n");
            goto err_ioctl_exit;
        }
        ilock->is_key_certified = true;
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_AUTHENTICATE_KEY:
        mutex_lock(&ilock->lock);
        if (ds28exx_authenticate_by_secret(ilock) != MATCH_MAC) {
            dev_err(ilock->dev, "IOCTL: failed to authenticate the key\n");
            goto err_ioctl_exit;
        }
        ilock->is_key_certified = true;
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_PWR_DOWN:
        mutex_lock(&ilock->lock);
        if (ilock->is_key_certified)
            gpio_direction_output(ilock->pdata->pwr_pin, !ilock->pdata->pwr_en_level);
        else {
            dev_err(ilock->dev, "IOCTL: no key is authenticated\n");
            goto err_ioctl_exit;
        }
        mutex_unlock(&ilock->lock);
        break;

    case LOCK_CYLINDER_IOC_PWR_ENABLE:
        mutex_lock(&ilock->lock);
        gpio_direction_output(ilock->pdata->pwr_pin, ilock->pdata->pwr_en_level);
        mutex_unlock(&ilock->lock);
        break;

    default:
        dev_err(ilock->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return 0;

err_ioctl_exit:
    mutex_unlock(&ilock->lock);
    return -EFAULT;
}

static struct file_operations lock_cylinder_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = lock_cylinder_dev_open,
    .release = lock_cylinder_dev_release,
    .unlocked_ioctl = lock_cylinder_dev_ioctl,
};

static void lock_cylinder_workhandler(struct work_struct *work)
{
    struct lock_cylinder_drvdata *ilock =
                container_of(work, struct lock_cylinder_drvdata, work);
    struct lock_cylinder_platform_data *pdata = ilock->pdata;
    char gpioval = gpio_get_value(ilock->pdata->detect_pin);

    if (ilock->is_key_inserted) {
        if (gpioval != pdata->inserted_level) {
            mutex_lock(&ilock->lock);
            ilock->is_wait_passed = true;
            ilock->is_key_inserted = false;
            ilock->is_key_certified = false;
            gpio_direction_output(pdata->pwr_pin, pdata->pwr_en_level);
            mutex_unlock(&ilock->lock);
            wake_up_interruptible(&ilock->wait);
        }
    } else {
        if (gpioval == pdata->inserted_level) {
            mutex_lock(&ilock->lock);
            ilock->is_wait_passed = true;
            ilock->is_key_inserted = true;
            mutex_unlock(&ilock->lock);
            wake_up_interruptible(&ilock->wait);
        }
    }
}

static void lock_cylinder_timerhandler(unsigned long data)
{
    struct lock_cylinder_drvdata *ilock = (struct lock_cylinder_drvdata *)data;

    /**
     * Start workqueue
     */
    schedule_work(&ilock->work);
}

static irqreturn_t lock_cylinder_key_detected_irqhandler(int irq, void *devid)
{
    struct lock_cylinder_drvdata *ilock = devid;

    disable_irq_nosync(ilock->detect_irq);

    /**
     * Set and start the timer
     */
    if (ilock->is_key_inserted)
        ilock->timer.expires = jiffies + HZ / 5;
    else
        ilock->timer.expires = jiffies + HZ / 3;

    mod_timer(&ilock->timer, ilock->timer.expires);

    enable_irq(ilock->detect_irq);
    return IRQ_HANDLED;
}

static int lock_cylinder_gpio_init(struct lock_cylinder_drvdata *ilock)
{
    struct lock_cylinder_platform_data *pdata = ilock->pdata;
    int error;

    if (gpio_is_valid(pdata->detect_pin)) {
        error = gpio_request(pdata->detect_pin, "detect_pin");
        if (error < 0) {
            dev_err(ilock->dev, "Failed to request GPIO%d, error %d\n", pdata->detect_pin, error);
            goto err_gpio_request1;
        }

        /**
         * Request key detected interrupt source
         */
        ilock->detect_irq = gpio_to_irq(pdata->detect_pin);
        if (ilock->detect_irq < 0) {
            error = ilock->detect_irq;
            dev_err(ilock->dev, "Unable to get irq number for GPIO%d, error %d\n",
                    pdata->detect_pin, error);
            goto err_irq_request1;
        }

        error = request_any_context_irq(ilock->detect_irq,                      \
                                        lock_cylinder_key_detected_irqhandler,  \
                                        IRQF_TRIGGER_FALLING | IRQF_DISABLED,   \
                                        dev_name(ilock->dev),                   \
                                        ilock);
        if (error < 0) {
            dev_err(ilock->dev, "Unable to clain irq %d, error %d\n", ilock->detect_irq, error);
            goto err_irq_request1;
        } else {
            enable_irq_wake(ilock->detect_irq);
        }
    } else {
        dev_err(ilock->dev, "Invalid detect_pin: %d\n", pdata->detect_pin);
        error = -ENODEV;
        goto err_gpio_request1;
    }

    if (gpio_is_valid(pdata->pwr_pin)) {
        error = gpio_request(pdata->pwr_pin, "pwr_pin");
        if (error < 0) {
            dev_err(ilock->dev, "Failed to request GPIO%d, error %d\n", pdata->pwr_pin, error);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->pwr_pin, pdata->pwr_en_level);
    } else {
        dev_err(ilock->dev, "Invalid pwr_pin: %d\n", pdata->pwr_pin);
        error = -ENODEV;
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->id_pin)) {
        error = gpio_request(pdata->id_pin, "id_pin");
        if (error < 0) {
            dev_err(ilock->dev, "Failed to request GPIO%d, error %d\n", pdata->id_pin, error);
            goto err_gpio_request3;
        }
        gpio_direction_output(pdata->id_pin, 1);
    } else {
        dev_err(ilock->dev, "Invalid id_pin: %d\n", pdata->id_pin);
        error = -ENODEV;
        goto err_gpio_request3;
    }

    return 0;

err_gpio_request3:
    gpio_free(pdata->pwr_pin);
err_gpio_request2:
    free_irq(ilock->detect_irq, ilock);
err_irq_request1:
    gpio_free(pdata->detect_pin);
err_gpio_request1:
    return error;
}

static void lock_cylinder_gpio_free(struct lock_cylinder_drvdata *ilock)
{
    struct lock_cylinder_platform_data *pdata = ilock->pdata;

    if (ilock->detect_irq)
        free_irq(ilock->detect_irq, ilock);

    gpio_free(pdata->detect_pin);
    gpio_free(pdata->pwr_pin);
    gpio_free(pdata->id_pin);
}

static int lock_cylinder_probe(struct platform_device *pdev)
{
    struct lock_cylinder_drvdata *ilock;
    int error;

    if (!pdev->dev.platform_data) {
        dev_err(&pdev->dev, "dev.platform_data cannot be NULL\n");
        error = -ENODEV;
        goto err_probe;
    }

    ilock = kzalloc(sizeof(struct lock_cylinder_drvdata), GFP_KERNEL);
    if (!ilock) {
        dev_err(&pdev->dev, "Failed to allocate drvdata memory\n");
        error = -ENOMEM;
        goto err_probe;
    }

    ilock->dev = &pdev->dev;
    ilock->pdata = pdev->dev.platform_data;

    ilock->miscdev.minor = MISC_DYNAMIC_MINOR;
    ilock->miscdev.name = pdev->name;
    ilock->miscdev.fops = &lock_cylinder_dev_fops;
    error = misc_register(&ilock->miscdev);
    if (error < 0) {
        dev_err(&pdev->dev, "Unable to register miscdevice, error %d\n", error);
        goto err_misc_register;
    }

    mutex_init(&ilock->lock);
    atomic_set(&ilock->opened, 0);
    init_timer(&ilock->timer);
    init_waitqueue_head(&ilock->wait);
    ilock->timer.data = (unsigned long)ilock;
    ilock->timer.function = lock_cylinder_timerhandler;

    INIT_WORK(&ilock->work, lock_cylinder_workhandler);

    if (lock_cylinder_gpio_init(ilock) < 0) {
        goto err_gpio_init;
    }

    ilock->is_set_secret = false;
    ilock->is_wait_passed = false;
    ilock->is_key_inserted = false;
    ilock->is_key_certified = false;

    platform_set_drvdata(pdev, ilock);
    dev_info(ilock->dev, "driver probe successful\n");
    return 0;

err_gpio_init:
    misc_deregister(&ilock->miscdev);
err_misc_register:
    kfree(ilock);
err_probe:
    return error;
}

static int lock_cylinder_remove(struct platform_device *pdev)
{
    struct lock_cylinder_drvdata *ilock = platform_get_drvdata(pdev);

    cancel_work_sync(&ilock->work);
    misc_deregister(&ilock->miscdev);
    lock_cylinder_gpio_free(ilock);
    kfree(ilock);
    return 0;
}

#ifdef CONFIG_LOCK_CYLINDER_PM
static int lock_cylinder_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct lock_cylinder_drvdata *ilock = platform_get_drvdata(pdev);

    mutex_lock(&ilock->lock);
    gpio_direction_output(ilock->pdata->pwr_pin, !ilock->pdata->pwr_en_level);
    mutex_unlock(&ilock->lock);
    return 0;
}

static int lock_cylinder_resume(struct platform_device *pdev)
{
    struct lock_cylinder_drvdata *ilock = platform_get_drvdata(pdev);

    mutex_lock(&ilock->lock);
    gpio_direction_output(ilock->pdata->pwr_pin, ilock->pdata->pwr_en_level);
    mutex_unlock(&ilock->lock);
    return 0;
}
#endif

static struct platform_driver lock_cylinder_driver = {
    .driver = {
        .name  = "lock_cylinder",
        .owner = THIS_MODULE,
    },
    .probe   = lock_cylinder_probe,
    .remove  = lock_cylinder_remove,
#ifdef CONFIG_LOCK_CYLINDER_PM
    .suspend = lock_cylinder_suspend,
    .resume  = lock_cylinder_resume,
#endif
};

static int __init lock_cylinder_init(void)
{
    return platform_driver_register(&lock_cylinder_driver);
}

static void __exit lock_cylinder_exit(void)
{
    platform_driver_unregister(&lock_cylinder_driver);
}

module_init(lock_cylinder_init);
module_exit(lock_cylinder_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("Lock cylinder driver");
