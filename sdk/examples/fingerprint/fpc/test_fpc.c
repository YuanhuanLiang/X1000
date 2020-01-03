#include <string.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <utils/file_ops.h>
#include <signal/signal_handler.h>
#include <power/power_manager.h>

#include <fingerprint/fpc/fpc_fingerprint.h>

#define LOG_TAG                         "test_fpc_fp"

#define UART_DEVICE_NAME                "/dev/ttyS1"

#define AUTH_MAX_TIMES                  5
#define MAX_ENROLL_TIMES                5
#define MAX_ENROLL_FINGER_NUM           200


#define AUTH_TIMEOUT                    5
#define ENROLL_TIMEOUT                  5

typedef enum {
    DELETE_BY_ID,
    DELETE_ALL,
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
static int auth_count;
static uint32_t fingers[32];
static int finger_count;
static customer_config_t fpc_finger_config;

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
    fprintf(stderr, "\n=================== FPC FP Menu ===================\n");
    fprintf(stderr, "  %d.Enroll\n", ENROLL_TEST);
    fprintf(stderr, "  %d.Authenticate\n", AUTH_TEST);
    fprintf(stderr, "  %d.List enrolled fingers\n", LIST_TEST);
    fprintf(stderr, "  %d.Delete\n", DELETE_TEST);
    fprintf(stderr, "  %d.Suspend\n", SUSPEND_TEST);
    fprintf(stderr, "  %d.Exit\n", EXIT_TEST);
    fprintf(stderr, "======================================================\n");
}

static void print_delete_tips(void) {
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please select delete type.\n");
    fprintf(stderr, "  %d.Delete by id\n", DELETE_BY_ID);
    fprintf(stderr, "  %d.Delete all\n", DELETE_ALL);
    fprintf(stderr, "==============================================\n");
}

static void print_delete_id_tips(void) {
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please enter finger id.\n");
    fprintf(stderr, "==============================================\n");
}

static void dump_fingers(void) {
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

    while (work_state == STATE_BUSY)
        pthread_cond_wait(&cond, &lock);

    pthread_mutex_unlock(&lock);
}

static void fpc_fp_callback(int msg, int percent, int finger_id)
{
    switch(msg)
    {
        case FPC_ENROLL_DUPLICATE:
            LOGW("Duplicate finger! Canceling...\n");
            set_state_idle();
            break;

        case FPC_ENROLL_ING:
            LOGD("========> FPC_ENROLL_ING , proc = %d\n", percent);
            LOGI("Finger enrolling finger good %d%%\n", percent);
            break;

        case FPC_ENROLL_SUCCESS:
            LOGD("========> FPC_ENROLL_SUCCESS , id = %d\n", finger_id);
            LOGI("Finger enrolling success\n");
            set_state_idle();
            break;

        case FPC_ENROLL_FAILED:
            LOGD("========> FPC_ENROLL_FAILED.\n");
            LOGI("Finger enrolling failed\n");
            set_state_idle();
            break;

        case FPC_ENROLL_TIMEOUT:
            LOGD("========> FPC_ENROLL_TIMEOUT\n");
            set_state_idle();
            break;

        case FPC_AUTHENTICATE_SUCCESS:
            LOGD("========> FP_MSG_MATCH_SUCESS , id = %d\n", finger_id);
            LOGI("Finger auth success, id=%d!\n", finger_id);
            auth_count = 0;
            set_state_idle();
            break;

        case FPC_AUTHENTICATED_FAILED:
            LOGD("========> FP_MSG_MATCH_FAILED , id = %d\n", finger_id);
            if (auth_count >= AUTH_MAX_TIMES - 1) {
                LOGE("Finger auth failure! Canceling...\n");
                fpc_fingerprint_cancel();
                auth_count = 0;
                set_state_idle();
            } else {
                fpc_fingerprint_authenticate();
                auth_count++;
                LOGE("Finger auth failure! %d try left\n", AUTH_MAX_TIMES - auth_count);
            }
            break;

        case FPC_AUTHENTICATED_TIMEOUT:
            LOGD("========> FPC_AUTHENTICATED_TIMEOUT.\n");
            set_state_idle();
            break;

        case FPC_REMOVE_SUCCESS:
            LOGD("========> FPC_REMOVE_SUCCESS\n");
            break;

        case FPC_REMOVE_FAILED:
            LOGD("========> FPC_REMOVE_FAILED\n");
            break;

        case FPC_LOW_QUALITY:
            LOGD("========> FPC_LOW_QUALITY\n");
            break;

        case FPC_CANCLED:
            LOGD("========> FPC_CANCLED\n");
            set_state_idle();
            break;

        default:
            break;
    }
}

static int do_enroll(void) {
    int error = 0;

    error = fpc_fingerprint_enroll();
    if (error < 0) {
        LOGE("Failedl to enroll fingerprint\n");
        return -1;
    }

    return 0;
}

static int do_auth(void) {
    int error = 0;

    finger_count = fpc_fingerprint_get_template_info(fingers);
    if (finger_count <= 0) {
        LOGW("No any valid finger's templete\n");
        return -1;
    }

    error = fpc_fingerprint_authenticate();
    if (error < 0) {
        LOGE("Failed to auth fingerprint\n");
        return -1;
    }

    return 0;
}

static int do_list(void) {
    finger_count = fpc_fingerprint_get_template_info(fingers);
    if (finger_count < 0) {
        LOGE("Failed to get template info\n");
        return -1;
    }

    dump_fingers();

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
    if (type > DELETE_ALL || type < 0)
        goto restart;

    if (type == DELETE_BY_ID) {
restart2:
        print_delete_id_tips();

        if (fgets(id_buf, sizeof(id_buf), stdin) == NULL)
            goto restart2;
        int id = strtol(id_buf, NULL, 0);

        error = fpc_fingerprint_delete(id, type);
        if (error < 0)
            LOGE("Failed to delete finger by id %d\n", id);

    } else if (type == DELETE_ALL) {
        error = fpc_fingerprint_delete(0, type);
        if (error < 0)
            LOGE("Failed to delete all finger\n");

    } else {
        LOGE("Invalid delete type\n");
        goto restart;
    }

    return error;
}

static int do_suspend(void) {
    int error = 0;

    power_manager->sleep();

    LOGI("=====> Wakeup <=====\n");

    error = fpc_fingerprint_reset();
    if (error)
        LOGE("Failed to reset fingerprint sensor\n");

    return 0;
}

static int do_exit(void) {
    int error = 0;

    error = fpc_fingerprint_destroy();
    if (error)
        LOGE("Failed to close microarray library\n");

    exit(error);
}

static void do_work(void) {
    int error = 0;
    char sel_buf[64] = {0};

    setbuf(stdin, NULL);

    do_list();

    for (;;) {
restart:
        wait_state_idle();

        print_tips();

        while (fgets(sel_buf, sizeof(sel_buf), stdin) == NULL);

        if (strlen(sel_buf) != 2)
            goto restart;

        int action = strtol(sel_buf, NULL, 0);
        if (action >= TEST_MAX || action < 0)
            goto restart;

        LOGI("Going to %s\n", test2string(action));

        if (action != LIST_TEST && action != EXIT_TEST && action != SUSPEND_TEST
                && action != DELETE_TEST)
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

static void handle_signal(int signal)
{
    fpc_fingerprint_cancel();
    fpc_fingerprint_destroy();

    exit(1);
}

int main(int argc, char *argv[]) {
    int error = 0;
    int retry_count = 0;

    power_manager = get_power_manager();
    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    fpc_finger_config.max_enroll_finger_num = MAX_ENROLL_FINGER_NUM;
    fpc_finger_config.enroll_timeout        = ENROLL_TIMEOUT;
    fpc_finger_config.authenticate_timeout  = AUTH_TIMEOUT;
    fpc_finger_config.uart_devname          = UART_DEVICE_NAME;
    memcpy(fpc_finger_config.file_path, get_user_system_dir(getuid()),
                                sizeof(fpc_finger_config.file_path));

reinit:
    error = fpc_fingerprint_init(fpc_fp_callback, &fpc_finger_config);
    if (error < 0) {
        LOGE("Failed to init fingerprint sensor\n");

        if (++retry_count > 3)
            return -1;

        goto reinit;
    }

    do_work();

    while (1)
        sleep(1000);

    return 0;
}
