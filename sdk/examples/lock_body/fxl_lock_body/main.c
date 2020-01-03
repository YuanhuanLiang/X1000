#include <unistd.h>
#include <utils/log.h>
#include <lock_body/fxl_lock_body/fxl_lock_body_manager.h>


#define LOG_TAG                         "fxl_lock_body_examples"

struct fxl_lock_body_manager* fxl_lock_body;


static char* tongue_str[] = {
    "FXL_STRETCH",
    "FXL_SHRINK",
    "FXL_SHAKE",
};

static void lock_body_status_cb(fxl_tongue_sta_t tongue)
{
    printf("bolique_tongue: %s, square_tongue: %s \n",
            tongue_str[tongue.bolique_tongue], tongue_str[tongue.square_tongue]);
}

int main()
{
    int ret;
    fxl_tongue_sta_t tongue = {0};

    fxl_lock_body = get_fxl_lock_body_manager();
    ret = fxl_lock_body->init(lock_body_status_cb);
    if (ret < 0) {
        LOGE("Failed to init\n");
    }

    fxl_lock_body->get_status(&tongue);
    printf("bolique_tongue: %s, square_tongue: %s \n",
            tongue_str[tongue.bolique_tongue], tongue_str[tongue.square_tongue]);
    while(getchar() != 'Q')
        printf("Enter 'Q' to exit\n");

    fxl_lock_body->deinit();
    return 0;
}