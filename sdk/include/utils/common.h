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

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <types.h>
#include <lib/libcommon.h>

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(a) ((sizeof(a)) / (sizeof(a[0])))
#endif

enum system_platform_t {
    XBURST,
    UNKNOWN,
};

#define _new(T, P)                          \
({                                          \
    T * obj = (T *)calloc(1, sizeof(T));    \
    obj->construct = construct_##P;         \
    obj->destruct = destruct_##P;           \
    obj->construct(obj);                    \
    obj;                                    \
})

#define _delete(P)                          \
({                                          \
    P->destruct(P);                         \
    free((void *)(P));                      \
})

char* get_process_name(pid_t pid);
char* get_current_process_name(void);
char* get_user_system_dir(uid_t uid);
void msleep(uint64_t msec);
void cold_boot(const char *path);
enum system_platform_t get_system_platform(void);

#endif /* COMMON_H */
