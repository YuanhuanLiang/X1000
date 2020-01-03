#include <unistd.h>
#include <utils/log.h>
#include <lock_body/mili_lock_body/mili_lock_body_manager.h>


#define LOG_TAG                         "mili_lock_body_examples"

struct mili_lock_body_manager* mili_lock_body;


static char* tongue_str[] = {
    "MILI_STRETCH",
    "MILI_SHRINK",
    "MILI_SHAKE",
    "MILI_ROTATE",
    "MILI_UNROTATE",
};

static void lock_body_status_cb(mili_tongue_sta_t tongue)
{
    printf("master_square: %s, anti_square: %s lock_cylinder: %s\n",
            tongue_str[tongue.master_square],
            tongue_str[tongue.anti_square],
            tongue_str[tongue.lock_cylinder]);
}

int main()
{
    int ret;
    mili_tongue_sta_t tongue = {0};

    mili_lock_body = get_mili_lock_body_manager();
    ret = mili_lock_body->init(lock_body_status_cb);
    if (ret < 0) {
        LOGE("Failed to init\n");
    }

    mili_lock_body->get_status(&tongue);

    while(getchar() != 'Q')
        printf("Enter 'Q' to exit\n");

    mili_lock_body->deinit();
    return 0;
}