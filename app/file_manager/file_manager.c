/*
 *  Copyright (C) 2017, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/file.h>

#include <file_manager.h>


/**
 * file_r_lock() 这样函数用于读取某个文件，读取过程加文件锁
 * @filename: 要读的文件名
 * @buffer: 存储读取文件内容的缓存区指针
 * @offset: 读取位置相对文件开头的偏移量
 * @count: 读取的字节数
 * 返回值: 成功读取到的字节数
 */
int file_r_lock(char *filename, char *buffer, int offset, int count)
{
    int fd;
    int retval;

    fd = open(filename, O_RDONLY | O_CREAT, 0644);
    if (fd < 0) {
        printf("%s: failed to open %s\n", __FUNCTION__, filename);
        return -1;
    }

    flock(fd, LOCK_EX); // 文件加锁

    lseek(fd, offset, SEEK_SET);
    retval = read(fd, buffer, count);

    close(fd);
    flock(fd, LOCK_UN); //释放文件锁

    return retval;
}

/**
 * file_w_lock() 这样函数用于写某个文件，写入过程加文件锁
 * @filename: 要写的文件名
 * @buffer: 存储写入文件内容的缓存区指针
 * @offset: 写入位置相对文件开头的偏移量
 * @count: 写入的字节数
 * 返回值: 成功写入到的字节数
 */
int file_w_lock(char *filename, char *buffer, int offset, int count)
{
    int fd;
    int retval;

    fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        printf("%s: failed to open %s\n", __FUNCTION__, filename);
        return -1;
    }

    flock(fd, LOCK_EX); // 文件加锁

    lseek(fd, offset, SEEK_SET);
    retval = write(fd, buffer, count);

    close(fd);
    flock(fd, LOCK_UN); //释放文件锁

    return retval;
}
