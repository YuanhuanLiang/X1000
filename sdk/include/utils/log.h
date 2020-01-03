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

#ifndef LOG_H_
#define LOG_H_

#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef LOCAL_DEBUG
#define DEBUG_TRACE 1
#else
#define DEBUG_TRACE 0
#endif

__attribute__((unused)) static pthread_mutex_t log_init_lock = PTHREAD_MUTEX_INITIALIZER;

#define TAG_BASE "libingenic--->"

#ifndef CONDITION
#define CONDITION(exp) __builtin_expect((exp) != 0, 0)
#endif

#ifdef DEBUG_TIMESTAMP
#define time_stamp()                                                           \
    do {                                                                       \
      struct timeval now_time;                                                 \
      struct tm *time_ptr;                                                     \
      gettimeofday(&now_time, NULL);                                           \
      time_ptr = localtime(&now_time.tv_sec);                                  \
      fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%.06ld:",                   \
            time_ptr->tm_year + 1900,                                          \
            time_ptr->tm_mon + 1,                                              \
            time_ptr->tm_mday,                                                 \
            time_ptr->tm_hour,                                                 \
            time_ptr->tm_min,                                                  \
            time_ptr->tm_sec,                                                  \
            now_time.tv_usec);                                                 \
    } while(0)

#else
#define time_stamp()                                                           \
    do {   } while(0)

#endif


#define LOGV(...)                                                              \
    do {                                                                       \
      int save_errno = errno;                                                  \
      pthread_mutex_lock(&log_init_lock);                                      \
      time_stamp();                                                            \
      fprintf(stderr, "V/%s%s %d: ", TAG_BASE, LOG_TAG, __LINE__);             \
      errno = save_errno;                                                      \
      fprintf(stderr, __VA_ARGS__);                                            \
      fflush(stderr);                                                          \
      pthread_mutex_unlock(&log_init_lock);                                    \
      errno = save_errno;                                                      \
    } while (0)

#define LOGD(...)                                                              \
    do {                                                                       \
      if (DEBUG_TRACE) {                                                       \
          int save_errno = errno;                                              \
          pthread_mutex_lock(&log_init_lock);                                  \
          time_stamp();                                                        \
          fprintf(stderr, "D/%s%s %d: ", TAG_BASE, LOG_TAG, __LINE__);         \
          errno = save_errno;                                                  \
          fprintf(stderr, __VA_ARGS__);                                        \
          fflush(stderr);                                                      \
          pthread_mutex_unlock(&log_init_lock);                                \
          errno = save_errno;                                                  \
      }                                                                        \
    } while (0)

#define LOGI(...)                                                              \
    do {                                                                       \
      int save_errno = errno;                                                  \
      pthread_mutex_lock(&log_init_lock);                                      \
      time_stamp();                                                            \
      fprintf(stderr, "I/%s%s %d: ", TAG_BASE, LOG_TAG, __LINE__);             \
      errno = save_errno;                                                      \
      fprintf(stderr, __VA_ARGS__);                                            \
      fflush(stderr);                                                          \
      pthread_mutex_unlock(&log_init_lock);                                    \
      errno = save_errno;                                                      \
    } while (0)

#define LOGW(...)                                                              \
    do {                                                                       \
      int save_errno = errno;                                                  \
      pthread_mutex_lock(&log_init_lock);                                      \
      time_stamp();                                                            \
      fprintf(stderr, "W/%s%s %d: ", TAG_BASE, LOG_TAG, __LINE__);             \
      errno = save_errno;                                                      \
      fprintf(stderr, __VA_ARGS__);                                            \
      fflush(stderr);                                                          \
      pthread_mutex_unlock(&log_init_lock);                                    \
      errno = save_errno;                                                      \
    } while (0)

#define LOGE(...)                                                              \
    do {                                                                       \
      int save_errno = errno;                                                  \
      pthread_mutex_lock(&log_init_lock);                                      \
      time_stamp();                                                            \
      fprintf(stderr, "E/%s%s %d: ", TAG_BASE, LOG_TAG, __LINE__);             \
      errno = save_errno;                                                      \
      fprintf(stderr, __VA_ARGS__);                                            \
      fflush(stderr);                                                          \
      pthread_mutex_unlock(&log_init_lock);                                    \
      errno = save_errno;                                                      \
    } while (0)

#define LOGV_IF(cond, ...)                                                     \
    if (CONDITION(cond))                                                       \
        LOGV(__VA_ARGS__)

#define LOGD_IF(cond, ...)                                                     \
    if (CONDITION(cond))                                                       \
        LOGD(__VA_ARGS__)

#define LOGI_IF(cond, ...)                                                     \
    if (CONDITION(cond))                                                       \
        LOGI(__VA_ARGS__)

#define LOGW_IF(cond, ...)                                                     \
    if (CONDITION(cond))                                                       \
        LOGW(__VA_ARGS__)

#define LOGE_IF(cond, ...)                                                     \
    if (CONDITION(cond))                                                       \
        LOGE(__VA_ARGS__)

#endif /* LOG_H_ */
