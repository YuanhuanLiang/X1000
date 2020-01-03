#include <string.h>
#include <sys/time.h>
#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <utils/file_ops.h>
#include <signal/signal_handler.h>
#include <power/power_manager.h>
#include <fingerprint/byd_fps.h>

#define LOG_TAG                         "test_byd_fp"

#define BYD_FPS_DEV_PATH                "/dev/byd_fps"

#define AUTH_MAX_TIMES                  5

#define ENROLL_TIOMEOUT                 10
#define MATCH_TIOMEOUT                  10
#define MAX_ENROLL_TIMES                3
#define MAX_ENROLL_FINGERS              100
#define BYD_FPS_MAX_RETRIES             1



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
static int fingers[MAX_ENROLL_FINGERS] = {0};
static int finger_count;


static const char* test2string(int item)
{
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

static void print_tips(void)
{
    fprintf(stderr, "\n=================== BYD FP Menu ===================\n");
    fprintf(stderr, "  %d.Enroll\n", ENROLL_TEST);
    fprintf(stderr, "  %d.Authenticate\n", AUTH_TEST);
    fprintf(stderr, "  %d.List enrolled fingers\n", LIST_TEST);
    fprintf(stderr, "  %d.Delete\n", DELETE_TEST);
    fprintf(stderr, "  %d.Suspend\n", SUSPEND_TEST);
    fprintf(stderr, "  %d.Exit\n", EXIT_TEST);
    fprintf(stderr, "======================================================\n");
}

static void print_delete_tips(void)
{
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please select delete type.\n");
    fprintf(stderr, "  %d.Delete by id\n", DELETE_BY_ID);
    fprintf(stderr, "  %d.Delete all\n", DELETE_ALL);
    fprintf(stderr, "==============================================\n");
}

static void print_delete_id_tips(void)
{
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please enter finger id.\n");
    fprintf(stderr, "==============================================\n");
}

static void dump_fingers(void)
{
    int i;
    fprintf(stderr, "\n============== Finger List ==============\n");
    for (i = 0; i < finger_count; i++)
        fprintf(stderr, "Finger[%d]: %d\n", i, fingers[i]);

    fprintf(stderr, "=========================================\n");
}

static void set_state_idle(void)
{
    pthread_mutex_lock(&lock);

    work_state = STATE_IDLE;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void set_state_busy(void)
{
    pthread_mutex_lock(&lock);

    work_state = STATE_BUSY;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void wait_state_idle(void)
{
    pthread_mutex_lock(&lock);

    while (work_state == STATE_BUSY)
        pthread_cond_wait(&cond, &lock);

    pthread_mutex_unlock(&lock);
}

static void byd_fp_callback(const byd_msg_t type, const uint32_t percent, const uint32_t finger)
{
    switch (type) {
    case BYD_MSG_FINGER_DOWN:
        LOGI("========> BYD_MSG_FINGER_DOWN <========\n");
        break;

    case BYD_MSG_FINGER_UP:
        LOGI("========> BYD_MSG_FINGER_UP <========\n");
        break;

    case BYD_MSG_PROCESS_SUCCESS:
        LOGI("========> BYD_MSG_PROCESS_SUCCESS <========\n");
        LOGI("Finger enrolling %d%%\n", percent);
        break;

    case BYD_MSG_PROCESS_FAILED:
        LOGI("========> BYD_MSG_PROCESS_FAILED <========\n");
        set_state_idle();
        break;

    case BYD_MSG_ENROLL_SUCCESS:
        LOGI("========> BYD_MSG_ENROLL_SUCCESS, fid = %d <========\n", finger);
        set_state_idle();
        break;

    case BYD_MSG_ENROLL_FAILED:
        LOGI("========> BYD_MSG_ENROLL_FAILED <========\n");
        set_state_idle();
        break;

    case BYD_MSG_MATCH_SUCESS:
        LOGI("========> BYD_MSG_MATCH_SUCESS: fid=%d\n", finger);
        set_state_idle();
        break;

    case BYD_MSG_MATCH_FAILED:
        LOGI("========> BYD_MSG_MATCH_FAILED <========\n");
        set_state_idle();
        break;

    case BYD_MSG_CANCEL:
        LOGI("========> BYD_MSG_CANCEL <========\n");
        break;


    case BYD_MSG_ENROLL_TIMEOUT:
        LOGI("========> BYD_MSG_ENROLL_TIMEOUT <========\n");
        set_state_idle();
        break;

    case BYD_MSG_MATCH_TIMEOUT:
        LOGI("========> BYD_MSG_MATCH_TIMEOUT <========\n");
        set_state_idle();
        break;

    case BYD_MSG_DELETE_SUCCESS:
        LOGI("========> BYD_MSG_DELETE_SUCCESS, fid = %d <========\n", finger);
        set_state_idle();
        break;

    case BYD_MSG_DELETE_FAILED:
        LOGI("========> BYD_MSG_DELETE_FAILED, fid = %d <========\n", finger);
        set_state_idle();
        break;

    case BYD_MSG_DUPLICATE_FINGER:
        LOGI("========> BYD_MSG_DUPLICATE_FINGER <========\n");
        LOGI("Duplicate finger\n");
        set_state_idle();
        break;

    case BYD_MSG_DUPLICATE_AREA:
        LOGI("========> BYD_MSG_DUPLICATE_AREA <========\n");
        break;

    case BYD_MSG_BAD_IMAGE:
        LOGI("========> BYD_MSG_BAD_IMAGE <========\n");
        break;

    default:
        LOGI("Invalid msg type: %d", type);
        break;
    }

}

static int do_enroll(void)
{
    int error = 0;

    error = fingerprint_enroll_byd(0,0);
    if (error < 0) {
        LOGI("Failedl to enroll fingerprint\n");
        return -1;
    }

    return 0;
}

static int do_auth(void)
{
    int error = 0;

    error = fingerprint_query_byd(1);
    if (error == 3) {
        LOGE("No any valid finger's templete\n");
        return -1;
    } else if(error == -255) {
        LOGE("already in auth fingerprint\n");
    }

    error = fingerprint_authenticate_byd(0, 0);
    if (error) {
        LOGE("already in auth fingerprint\n");
        return -1;
    }

    return 0;
}

static int do_list(void) {
    finger_count = byd_get_fingerprint_id_info(fingers);
    dump_fingers();

    return 0;
}

static int do_delete(void)
{
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

        error = fingerprint_remove_byd(0,id);
        if (error < 0)
            LOGE("Failed to delete finger by id %d\n", id);

    } else if (type == DELETE_ALL) {
        error = fingerprint_remove_byd(0,0);
        if (error < 0)
            LOGE("Failed to delete all finger\n");

    } else {
        LOGE("Invalid delete type\n");
        goto restart;
    }

    return error;
}

static int do_suspend(void)
{

    power_manager->sleep();
    LOGI("=====> Wakeup <=====\n");

    return 0;
}

static int do_exit(void)
{
    int error = 0;

    error = fingerprint_close_byd();
    if (error)
        LOGE("Failed to close microarray library\n");

    exit(error);
}

static void do_work(void)
{
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
            LOGE("Failed to %s\n", test2string(action));
            set_state_idle();
        }
    }
}

static void handle_signal(int signal)
{
    fingerprint_close_byd();
    exit(1);
}

int main(int argc, char *argv[]) {
    int error = 0;
    char path[128] = {0};

    power_manager = get_power_manager();
    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    sprintf(path,"/%s/",get_user_system_dir(getuid()));
    error = set_notify_callback(byd_fp_callback);
    if (error < 0) {
        LOGE("Failed to set byd fp callback\n");
        return -1;
    }

    error = fingerprint_get_id_byd(BYD_FPS_DEV_PATH, MAX_ENROLL_FINGERS, MAX_ENROLL_TIMES, BYD_FPS_MAX_RETRIES,
                                   ENROLL_TIOMEOUT, MATCH_TIOMEOUT, path);
    if (error) {
        LOGE("Failed to init byd lib\n");
        return -1;
    }


    do_work();

    while (1)
        sleep(1000);

    return 0;
}
