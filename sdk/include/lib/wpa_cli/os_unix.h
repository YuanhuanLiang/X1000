/*
 *  Copyright (C) 2017, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 * This software may be distributed, used, and modified under the terms of
 * BSD license:

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 *  You should have received a copy of the BSD License along with this program.
 *
 */
#ifndef _OS_UNIX_H
#define _OS_UNIX_H

#define os_snprintf snprintf
#define os_memcmp(s1, s2, n) memcmp((s1), (s2), (n))
#define os_strlen(s) strlen(s)
#define os_strdup(s) strdup(s)
#define os_strcmp(s1, s2) strcmp((s1), (s2))
#define os_strstr(h, n) strstr((h), (n))
#define os_strchr(s, c) strchr((s), (c))
#define os_strrchr(s, c) strrchr((s), (c))

#define os_malloc(s) malloc((s))

void *os_zalloc(size_t size);
void os_sleep(long sec, long usec);
int os_strlcpy(char *dest, const char *src, int size);
int os_get_reltime(struct timeval *t);

static inline void os_reltime_sub(struct timeval *a, struct timeval *b,
                  struct timeval *res)
{
    res->tv_sec = a->tv_sec - b->tv_sec;
    res->tv_usec = a->tv_usec - b->tv_usec;
    if (res->tv_usec < 0) {
        res->tv_sec--;
        res->tv_usec += 1000000;
    }
}

static inline int os_reltime_expired(struct timeval *now,
                struct timeval *ts,
                long timeout_secs)
{
    struct timeval age;

    os_reltime_sub(now, ts, &age);
    return (age.tv_sec > timeout_secs) ||
        (age.tv_sec == timeout_secs && age.tv_usec > 0);
}

static inline int os_snprintf_fail(size_t size, int ret)
{
    return ret < 0 || (unsigned int)ret >= size;
}

#endif /* _OS_UNIX_H */
