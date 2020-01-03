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

#ifndef LIB_I2C_SMBUS_H
#define LIB_I2C_SMBUS_H

#include <linux/i2c.h>

extern int i2c_smbus_access(int fd, char read_write, unsigned char cmd,
        int size, union i2c_smbus_data *data);

extern int i2c_smbus_write_quick(int fd, unsigned char value);
extern int i2c_smbus_read_byte(int fd);
extern int i2c_smbus_write_byte(int fd, unsigned char value);
extern int i2c_smbus_read_byte_data(int fd, unsigned char cmd);
extern int i2c_smbus_write_byte_data(int fd, unsigned char cmd,
        unsigned char value);
extern int i2c_smbus_read_word_data(int fd, unsigned char cmd);
extern int i2c_smbus_write_word_data(int fd, unsigned char cmd,
        unsigned short value);
extern int i2c_smbus_process_call(int fd, unsigned char cmd,
        unsigned short value);

/* Returns the number of read bytes */
extern int i2c_smbus_read_block_data(int fd, unsigned char cmd,
        unsigned char *values);
extern int i2c_smbus_write_block_data(int fd, unsigned char cmd,
        unsigned char length, const unsigned char *values);

/* Returns the number of read bytes */
/* Until kernel 2.6.22, the length is hardcoded to 32 bytes. If you
 ask for less than 32 bytes, your code will only work with kernels
 2.6.23 and later. */
extern int i2c_smbus_read_i2c_block_data(int fd, unsigned char cmd,
        unsigned char length, unsigned char *values);
extern int i2c_smbus_write_i2c_block_data(int fd, unsigned char cmd,
        unsigned char length, const unsigned char *values);

/* Returns the number of read bytes */
extern int i2c_smbus_block_process_call(int fd, unsigned char cmd,
        unsigned char length, unsigned char *values);

extern int i2c_smbus_open(int i2cbus);
extern int i2c_smbus_close(int fd);
extern int i2c_smbus_get_funcs_matrix(int fd, unsigned long *funcs);
extern int i2c_smbus_set_slave_addr(int fd, int addr, bool force);

#endif /* LIB_I2C_SMBUS_H */
