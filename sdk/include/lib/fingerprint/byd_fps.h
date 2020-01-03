/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef BYD_FPS_H_
#define BYD_FPS_H_
#include <stdlib.h>
#include "stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "stdbool.h"
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>

/*
 * Definitions
 */
#define ALOGD   printf
#define ALOGE   printf

#ifdef __cplusplus
#define C_FILE_START  extern "C" {
#define C_FILE_END    }
#define EXTERNC       extern "C"
#else
#define C_FILE_START
#define C_FILE_END
#define EXTERNC
#endif



typedef enum {
    BYD_MSG_FINGER_DOWN          = 0,       /*  */
    BYD_MSG_FINGER_UP            = 1,       /* reserved */
    BYD_MSG_PROCESS_SUCCESS      = 2,
    BYD_MSG_PROCESS_FAILED       = 3,
    BYD_MSG_ENROLL_SUCCESS       = 4,
    BYD_MSG_ENROLL_FAILED        = 5,
    BYD_MSG_MATCH_SUCESS         = 6,
    BYD_MSG_MATCH_FAILED         = 7,
    BYD_MSG_CANCEL               = 8,
    BYD_MSG_ENROLL_TIMEOUT       = 9,
    BYD_MSG_MATCH_TIMEOUT        = 10,
    BYD_MSG_DELETE_SUCCESS       = 11,
    BYD_MSG_DELETE_FAILED        = 12,
    BYD_MSG_DUPLICATE_FINGER     = 13,
    BYD_MSG_DUPLICATE_AREA       = 14,
    BYD_MSG_BAD_IMAGE            = 15,
    BYD_MSG_MAX
} byd_msg_t;

typedef enum {
    BYD_DELETE_BY_ID,
    BYD_DELETE_ALL,
} byd_del_type_t;

typedef struct fingerprint_msg {
    byd_msg_t type;
    uint32_t percent;
    uint32_t finger;
} fingerprint_msg_t;


#define FPS_MAX_RESULT_BUFFER_SIZE        12
#define FPS_MAX_SEND_DATA_SIZE            6
#define FPS_MAX_SEND_DATA_SIZE_DRIVER     FPS_MAX_SEND_DATA_SIZE

#define BYD_FPS_FINGER_DETECTED           0
#define BYD_FPS_DRIVER_INIT               1
#define BYD_FPS_REPORT_KEY                2
#define BYD_FPS_GET_INTR_TYPE             3
#define BYD_FPS_KEY_FUNCTION              4 //for app special
#define BYD_FPS_CAPTURE_ONCE              5
#define BYD_FPS_CHIP_INIT                 6
#define BYD_FPS_NOTIFY_DEV_APK            7
#define BYD_FPS_WAIT_FINGER_DOWN_8        8
#define BYD_FPS_ABORT                     9
#define BYD_FPS_WAIT_FINGER_DOWN          12
#define BYD_FPS_DELETE_TEMPLATE           13
#define BYD_FPS_IS_FINGER_EXIST           14
#define BYD_FPS_READ_IMAGE                15
#define BYD_FPS_READ_ALG_RESULT           16
#define END_CMD_DO_NOTHING                0xFF

/* op_flag to StartCaptureOnce */
#define BYD_FINGER_CAPTURE                0
#define BYD_FINGER_ENROLL                 1
#define BYD_FINGER_VERIFY                 2
#define BYD_FINGER_VERIFY_HIGH_SECURITY   3
#define BYD_FINGER_PRE_ENROLL_TOKEN       4
#define BYD_FINGER_ENROLL_TOKEN           5
#define BYD_FINGER_VERIFY_TOKEN           6
#define BYD_FINGER_VERIFY_FIOD_TOKEN      7
#define BYD_FINGER_GET_ADJUST             8

/* is_read to StartCaptureOnce */
#define READ_NO_IMAGE_DATA                          0
#define READ_IMAGE_DATA                             1
#define READ_NO_IMAGE_DATA_AND_NOT_DETECT_FINGER    2
#define READ_IMAGE_DATA_AND_NOT_DETECT_FINGER       3 //almost same with READ_IMAGE_DATA because of driver

/* timeout when finger is not tip on */
#define TIMEOUT              10

#define READ_FINGER_SATTE    2
#define READ_SENSOR_MODE     3
#define READ_ENROL_RESULT    5
#define READ_MATCH_RESULT    6


/* Callback function type */
typedef void (*fingerprint_notify_t)(const byd_msg_t type, const uint32_t percent, const uint32_t finger);
int fingerprint_enroll_byd(uint32_t gid, uint32_t timeout_sec);
int fingerprint_cancel_byd();
int fingerprint_remove_byd(uint32_t gid, uint32_t fid);
int fingerprint_authenticate_byd(uint64_t operation_id, uint32_t gid);
int set_notify_callback(fingerprint_notify_t notify);
int fingerprint_query_byd(uint32_t fingerprint_id);
int byd_get_fingerprint_id_info(int id_info[]);
int fingerprint_close_byd();
int fingerprint_get_id_byd(const char *device_name, const int finger_num, const int enroll_count, const int max_verify_retry,
                        const int enroll_timeout, const int match_timeout, const char* tmeplate_path);

#endif // BYD_FPS_H_
