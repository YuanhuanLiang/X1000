/*
* @Author: AK
* @Date:   2017-07-14 10:27:58
* @Last Modified by:   AK
* @Last Modified time: 2017-07-17 16:50:12
*/
#ifndef __MA_FINGERPRINT__
#define __MA_FINGERPRINT__
#include <stdint.h>
typedef enum fingerprint_msg_type {
    FINGERPRINT_ERROR = -1,
    FINGERPRINT_ACQUIRED = 1,
    FINGERPRINT_TEMPLATE_ENROLLING = 3,
    FINGERPRINT_TEMPLATE_REMOVED = 4,
    FINGERPRINT_AUTHENTICATED = 5
} fingerprint_msg_type_t;

typedef enum fingerprint_error {
    FINGERPRINT_ERROR_HW_UNAVAILABLE = 1, /* The hardware has an error that can't be resolved. */
    FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2, /* Bad data; operation can't continue */
    FINGERPRINT_ERROR_TIMEOUT = 3, /* The operation has timed out waiting for user input. */
    FINGERPRINT_ERROR_NO_SPACE = 4, /* No space available to store a template */
    FINGERPRINT_ERROR_CANCELED = 5, /* The current operation can't proceed. See above. */
    FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6, /* fingerprint with given id can't be removed */
    FINGERPRINT_ERROR_VENDOR_BASE = 1000 /* vendor-specific error messages start here */
} fingerprint_error_t;


typedef enum fingerprint_acquired_info {
    FINGERPRINT_ACQUIRED_GOOD = 0,
    FINGERPRINT_ACQUIRED_PARTIAL = 1, /* sensor needs more data, i.e. longer swipe. */
    FINGERPRINT_ACQUIRED_INSUFFICIENT = 2, /* image doesn't contain enough detail for recognition*/
    FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
    FINGERPRINT_ACQUIRED_TOO_SLOW = 4, /* mostly swipe-type sensors; not enough data collected */
    FINGERPRINT_ACQUIRED_TOO_FAST = 5, /* for swipe and area sensors; tell user to slow down*/
    FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000, /* vendor-specific acquisition messages start here */
    FINGERPRINT_ACQUIRED_ALI_BASE = 1100,
    FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = FINGERPRINT_ACQUIRED_ALI_BASE + 1,
    FINGERPRINT_ACQUIRED_FINGER_DOWN = FINGERPRINT_ACQUIRED_ALI_BASE + 2,
    FINGERPRINT_ACQUIRED_FINGER_UP = FINGERPRINT_ACQUIRED_ALI_BASE + 3,
    FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = FINGERPRINT_ACQUIRED_ALI_BASE + 4,
    FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = FINGERPRINT_ACQUIRED_ALI_BASE + 5,
    FINGERPRINT_ACQUIRED_DUPLICATE_AREA = FINGERPRINT_ACQUIRED_ALI_BASE + 6,
    FINGERPRINT_ACQUIRED_LOW_COVER = FINGERPRINT_ACQUIRED_ALI_BASE + 7,
    FINGERPRINT_ACQUIRED_BAD_IMAGE = FINGERPRINT_ACQUIRED_ALI_BASE + 8,
    FINGERPRINT_ACQUIRED_TRIAL_OVER = FINGERPRINT_ACQUIRED_ALI_BASE + 9,
} fingerprint_acquired_info_t;


typedef struct fingerprint_finger_id {
    uint32_t gid;
    uint32_t fid;
} fingerprint_finger_id_t;

typedef struct fingerprint_enroll {
    fingerprint_finger_id_t finger;
    /* samples_remaining goes from N (no data collected, but N scans needed)
     * to 0 (no more data is needed to build a template). */
    uint32_t samples_remaining;
    uint64_t msg; /* Vendor specific message. Used for user guidance */
} fingerprint_enroll_t;

typedef struct fingerprint_removed {
    fingerprint_finger_id_t finger;
} fingerprint_removed_t;

typedef struct fingerprint_acquired {
    fingerprint_acquired_info_t acquired_info; /* information about the image */
} fingerprint_acquired_t;

typedef struct fingerprint_authenticated {
    fingerprint_finger_id_t finger;
} fingerprint_authenticated_t;

typedef struct fingerprint_msg {
    fingerprint_msg_type_t type;
    union {
        fingerprint_error_t error;
        fingerprint_enroll_t enroll;
        fingerprint_removed_t removed;
        fingerprint_acquired_t acquired;
        fingerprint_authenticated_t authenticated;
    } data;
} fingerprint_msg_t;

#define FINGERPRINT_LIMIT 20
#define VERSION_TRAIL    0
#define VERSION_RELEASE  1

/* Callback function type */
typedef void (*fingerprint_notify_t)(const fingerprint_msg_t *msg);


int ma_fingerprint_set_enroll_times(int times);

int ma_fingerprint_set_max_fingers(int fingers);

int ma_fingerprint_enroll();

int ma_fingerprint_cancel();

int ma_fingerprint_remove(uint32_t fid);

int ma_fingerprint_authenticate();

int ma_fingerprint_set_notify_callback(fingerprint_notify_t notify);

int ma_fingerprint_enumerate(fingerprint_finger_id_t *results,
        uint32_t *max_size);

int ma_fingerprint_open(const char *store_path);

int ma_fingerprint_close();

int ma_fingerprint_get_version(char *version, int size);

#endif
