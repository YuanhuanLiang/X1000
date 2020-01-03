#include <unistd.h>
#include <utils/log.h>
#include <sgm42507/sgm42507_manager.h>

#define LOG_TAG                         "sgm42507_examples"

struct sgm42507_manager* sgm42507;
int main()
{
    int ret;

    sgm42507 = get_sgm42507_manager();
    ret = sgm42507->init();
    if (ret < 0) {
        LOGE("Failed init\n");
        return -1;
    }

    ret = sgm42507->set_power(SGM42507_POWER_ON);
    if (ret < 0) {
        LOGE("Failed set power\n");
        return -1;
    }

    ret = sgm42507->drv_enable(SGM42507_DRI_ENABLE);
    if (ret < 0) {
        LOGE("Failed enable driver\n");
        return -1;
    }

    ret = sgm42507->set_direction(SGM42507_DIRECTION_A);
    if (ret < 0) {
        LOGE("Failed set direction A\n");
        return -1;
    }

    usleep(500*1000);
    ret = sgm42507->drv_enable(SGM42507_DRI_DISABLE);
    if (ret < 0) {
        LOGE("Failed enable driver\n");
        return -1;
    }

    usleep(5*1000*1000);

    ret = sgm42507->drv_enable(SGM42507_DRI_ENABLE);
    if (ret < 0) {
        LOGE("Failed enable driver\n");
        return -1;
    }

    ret = sgm42507->set_direction(SGM42507_DIRECTION_B);
    if (ret < 0) {
        LOGE("Failed set direction B\n");
        return -1;
    }

    usleep(500*1000);
    ret = sgm42507->drv_enable(SGM42507_DRI_DISABLE);
    if (ret < 0) {
        LOGE("Failed enable driver\n");
        return -1;
    }

    ret = sgm42507->set_power(SGM42507_POWER_OFF);
    if (ret < 0) {
        LOGE("Failed set power\n");
        return -1;
    }

    sgm42507->deinit();
    return 0;
}