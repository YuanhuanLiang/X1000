#include <string.h>
#include <limits.h>
#include <stdio.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <utils/file_ops.h>
#include <signal/signal_handler.h>
#include <utils/assert.h>
#include <power/power_manager.h>
#include <lib/fingerprint/mafp_intf.h>

#define LOG_TAG "test_microarray_fp"

#define ENROLL_TIMES                    6
#define AUTH_MAX_TIMES                  5

#define MAX_ENROLL_TIMEOUT              5
#define MAX_AUTH_TIMEOUT                5
#define MAX_ENROLL_FINGERS              100

typedef enum {
    DELETE_ALL,
    DELETE_BY_ID,
} delete_type_t;

enum {
    ENROLL_TEST,
    AUTH_TEST,
    LIST_TEST,
    DELETE_TEST,
    SUSPEND_TEST,
    EXIT_TEST,
    TEST_MAX,
};

enum {
    STATE_IDLE,
    STATE_BUSY
};

static struct power_manager* power_manager;
static struct signal_handler* signal_handler;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int work_state = STATE_IDLE;
static int enroll_steps;
static int auth_count;
static int fingers[MAX_ENROLL_FINGERS];


static const char* test2string(int item) {
    switch (item) {
    case ENROLL_TEST:
        return "enroll test";
    case AUTH_TEST:
        return "authenticate test";
    case LIST_TEST:
        return "list fingers test";
    case DELETE_TEST:
        return "delete test";
    case SUSPEND_TEST:
        return "suspend test";
    case EXIT_TEST:
        return "exit";
    default:
        return "unknown test";
    }
}

static void print_tips(void) {
    fprintf(stderr, "\n================= Microarray FP Menu =================\n");
    fprintf(stderr, "  %d.Enroll\n", ENROLL_TEST);
    fprintf(stderr, "  %d.Authenticate\n", AUTH_TEST);
    fprintf(stderr, "  %d.List enrolled fingers\n", LIST_TEST);
    fprintf(stderr, "  %d.Delete\n", DELETE_TEST);
    fprintf(stderr, "  %d.Suspend\n", SUSPEND_TEST);
    fprintf(stderr, "  %d.Exit\n", EXIT_TEST);
    fprintf(stderr, "======================================================\n");
}

static void print_delete_tips(void) {
    fprintf(stderr, "\n==============================================\n");
    fprintf(stderr, "Please select delete type.\n");
    fprintf(stderr, "  %d.Delete all\n", DELETE_ALL);
    fprintf(stderr, "  %d.Delete by id\n", DELETE_BY_ID);
    fprintf(stderr, "==============================================\n");
}

static void print_delete_id_tips(void) {
    fprintf(stderr, "\n==============================================\n");
    fprintf(stderr, "Please enter finger id.\n");
    fprintf(stderr, "==============================================\n");
}

static void dump_fingers(int finger_count) {
    fprintf(stderr, "\n============== Finger List ==============\n");
    for (int i = 0; i < finger_count; i++)
        fprintf(stderr, "Finger[%d]: %d\n", i, fingers[i]);

    fprintf(stderr, "=========================================\n");
}

static void set_state_idle(void) {
    pthread_mutex_lock(&lock);

    work_state = STATE_IDLE;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void set_state_busy(void) {
    pthread_mutex_lock(&lock);

    work_state = STATE_BUSY;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void wait_state_idle(void) {
    pthread_mutex_lock(&lock);

    while (work_state == STATE_BUSY) {
        pthread_cond_wait(&cond, &lock);
    }

    pthread_mutex_unlock(&lock);
}

static void ma_fp_callback(int msg, int percent, int finger_id) {
    int error;
    switch (msg) {
    case MAFP_MSG_FINGER_DOWN:
        LOGI("========> MAFP_MSG_FINGER_DOWN\n");
        break;

    case MAFP_MSG_FINGER_UP:
        LOGI("========> MAFP_MSG_FINGER_UP\n");
        break;

    case MAFP_MSG_NO_LIVE:
        LOGI("========> MAFP_MSG_NO_LIVE\n");
        set_state_idle();
        break;

    case MAFP_MSG_PROCESS_SUCCESS:
        LOGI("========> MAFP_MSG_PROCESS_SUCCESS: acquired_info=%d\n", percent);
        break;

    case MAFP_MSG_PROCESS_FAILED:
        LOGW("========> MAFP_MSG_PROCESS_SUCCESS: acquired_info=%d\n", percent);
        set_state_idle();
        break;

    case MAFP_MSG_DUPLICATE_FINGER:
        LOGW("========> MAFP_MSG_PROCESS_DUPLICATE id:%d\n",finger_id);
        set_state_idle();
        break;

    case MAFP_MSG_ENROLL_SUCCESS:
        LOGI("========> MAFP_MSG_ENROLL_SUCCESS, id: %d\n",finger_id);
        set_state_idle();
        break;

    case MAFP_MSG_ENROLL_FAILED:
        LOGI("========> MAFP_MSG_ENROLL_FAILED\n");
        set_state_idle();
        break;

    case MAFP_MSG_MATCH_SUCESS:
        LOGI("========> MAFP_MSG_MATCH_SUCESS\n");
        LOGI("Finger auth success, id=%d!\n", finger_id);
        auth_count = 0;
        set_state_idle();
        break;

    case MAFP_MSG_MATCH_FAILED:
        LOGW("========> MAFP_MSG_MATCH_FAILED\n");
        if (auth_count >= AUTH_MAX_TIMES - 1) {
            LOGE("Finger auth failure!\n");
            auth_count = 0;
            set_state_idle();
        } else {
            auth_count++;
            LOGE("Finger auth failure! %d try left\n", AUTH_MAX_TIMES - auth_count);
            error = mafp_api_authenticate();
            if (error) {
                LOGE("Failed to auth fingerprint\n");
                return;
            }
        }
        break;

    case MAFP_MSG_DELETE_SUCCESS:
        LOGD("========> MAFP_MSG_DELETE_SUCCESS\n");
        LOGI("Finger removed, id=%d\n", finger_id);
        set_state_idle();
        break;

    case MAFP_MSG_DELETE_FAILED:
        LOGD("========> MAFP_MSG_DELETE_FAILED\n");
        LOGI("Finger removed, id=%d\n", finger_id);
        set_state_idle();
        break;

    case MAFP_MSG_CANCEL:
        LOGI("========> MAFP_MSG_CANCEL\n");
        // set_state_idle();
        break;

    case MAFP_MSG_ENROLL_TIMEOUT:
        LOGI("========> MAFP_MSG_ENROLL_TIMEOUT\n");
        set_state_idle();
        break;

    case MAFP_MSG_MATCH_TIMEOUT:
        LOGI("========> MAFP_MSG_MATCH_TIMEOUT\n");
        set_state_idle();
        break;

    case MAFP_MSG_BAD_IMAGE:
        LOGI("========> MAFP_MSG_BAD_IMAGE\n");
        break;
    default:
        LOGE("Invalid msg type: %d\n", msg);
        break;
    }
}

static void handle_signal(int signal) {
    int error = 0;

    error = mafp_api_destroy();
    if (error)
        LOGE("Failed to close microarray library\n");

    exit(1);
}

static int do_enroll(void) {
    int error = 0;

    enroll_steps = -1;

    error = mafp_api_get_template_info(fingers);
    if (error < 0) {
        LOGE("Failed to enumerate fingers\n");
        return -1;
    }

    if(error > MAX_ENROLL_FINGERS) {
        LOGE("Failed to enumerate fingers\n");
        return -1;
    }

    error = mafp_api_enroll();
    if (error) {
        LOGE("Failedl to enroll fingerprint, fingers can't more than %d\n",
                                                        MAX_ENROLL_FINGERS);
        return -1;
    }

    return 0;
}

static int do_auth(void) {
    int error = 0;
    error = mafp_api_get_template_info(fingers);
    if (error < 0) {
        LOGE("Failed to enumerate fingers\n");
        return -1;
    }

    assert_die_if(error > MAX_ENROLL_FINGERS, "Invalid finger size\n");

    if (error <= 0) {
        LOGW("No any valid finger's templete\n");
        return -1;
    }

    error = mafp_api_authenticate();
    if (error) {
        LOGE("Failed to auth fingerprint\n");
        return -1;
    }

    return 0;
}

static int do_list(void) {
    int error = 0;

    error = mafp_api_get_template_info(fingers);
    if (error < 0) {
        LOGE("Failed to enumerate fingers\n");
        return -1;
    }

    dump_fingers(error);

    return 0;
}

static int do_delete(void) {
    int error = 0;
    char id_buf[128] = {0};
    char type_buf[32] = {0};

restart:
    print_delete_tips();
    if (fgets(type_buf, sizeof(type_buf), stdin) == NULL)
        goto restart;
    if (strlen(type_buf) != 2)
        goto restart;

    int type = strtol(type_buf, NULL, 0);
    if (type > DELETE_BY_ID || type < 0)
        goto restart;

    if (type == DELETE_BY_ID) {
restart2:
        print_delete_id_tips();

        if (fgets(id_buf, sizeof(id_buf), stdin) == NULL)
            goto restart2;
        uint32_t id = strtol(id_buf, NULL, 0);

        error = mafp_api_delete(id,0);
        if (error)
            LOGE("Failed to delete finger by id %d\n", id);

    } else if (type == DELETE_ALL) {
        error = mafp_api_delete(0,1);
        if (error)
            LOGE("Failed to delete all finger\n");
    }

    return error;
}

static int do_suspend(void) {
    power_manager->sleep();

    LOGI("=====> Wakeup <=====\n");

    return 0;
}

static int do_exit(void) {
    int error = 0;

    error = mafp_api_destroy();
    if (error)
        LOGE("Failed to close microarray library\n");

    exit(error);
}

static void do_work(void) {
    int error = 0;
    char sel_buf[64] = {0};
    int action;

    setbuf(stdin, NULL);

    do_list();

    for (;;) {
restart:
        wait_state_idle();
        print_tips();

        while (fgets(sel_buf, sizeof(sel_buf), stdin) == NULL);

        if (strlen(sel_buf) != 2)
            goto restart;

        action = strtol(sel_buf, NULL, 0);
        if (action >= TEST_MAX || action < 0)
            goto restart;

        LOGI("Going to %s\n", test2string(action));

        if (action != LIST_TEST && action != EXIT_TEST && action != SUSPEND_TEST)
            set_state_busy();

        switch(action) {
        case ENROLL_TEST:
            error = do_enroll();
            break;

        case AUTH_TEST:
            error = do_auth();
            break;

        case LIST_TEST:
            error = do_list();
            break;

        case DELETE_TEST:
            error = do_delete();
            break;

        case SUSPEND_TEST:
            error = do_suspend();
            break;

        case EXIT_TEST:
            error = do_exit();
            break;

        default:
            break;
        }

        if (error) {
            LOGI("Failed to %s\n", test2string(action));
            set_state_idle();
        }
    }
}

int main(int argc, char *argv[]) {
    int error = 0;

    power_manager = get_power_manager();
    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    error = mafp_api_init(get_user_system_dir(getuid()), ma_fp_callback);
    if (error) {
        LOGE("Failed to init mafp\n");
        return -1;
    }

    error =  mafp_api_set_enroll_timeout(MAX_ENROLL_TIMEOUT);
    if (error) {
        LOGE("Failed to set enroll timeout\n");
    }

    error =  mafp_api_set_authenticate_timeout(MAX_AUTH_TIMEOUT);
    if (error) {
        LOGE("Failed to set enroll timeout\n");
    }

    error = mafp_api_set_enroll_times(ENROLL_TIMES);
    if (error) {
        LOGE("Failed to set enroll timeout\n");
    }
    do_work();

    while (1)
        sleep(1000);

    return 0;
}
