#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <types.h>
#include <utils/log.h>
#include <vibrator/vibrator_manager.h>
#include <netlink/netlink_manager.h>

#define LOG_TAG "test_vibrator"

static void motor_event_callback(uint32_t motor_id, motor_status status) {
    LOGI("motor%d event callback status: %d\n", motor_id, status);
}

int main(int argc, char *argv[]) {
    int id;
    struct vibrator_manager* vibrator_manager = get_vibrator_manager();
    struct netlink_manager* netlink_manager = get_netlink_manager();

    if(argc != 2) {
        LOGE("Use: %s [ motor id ]\n", argv[0]);
        return -1;
    }
    id = atoi(argv[1]);

    if (vibrator_manager->init()) {
        LOGE("Failed to init vibrator manager.\n");
        return -1;
    }

    if (netlink_manager->init()) {
        LOGE("Failed to init netlink manager.\n");
        vibrator_manager->deinit();
        return -1;
    }

    netlink_manager->register_handler(vibrator_manager->get_netlink_handler());

    if (netlink_manager->start()) {
        LOGE("Failed to start netlink manager.\n");
    }

    if(vibrator_manager->open(id)) {
        LOGE("open motor%d error\n", id);
        vibrator_manager->deinit();
        netlink_manager->deinit();
        return -1;
    }

    if(vibrator_manager->register_event_callback(id, motor_event_callback)) {
        LOGE("motor%d set_event_callback error\n", id);
    }

    if(vibrator_manager->set_cycle(id, 100)) {
        LOGE("motor%d set_cycle error\n", id);
    }
    if(vibrator_manager->set_speed(id, 5)) {
        LOGE("motor%d set_speed error\n", id);
    }
    if(vibrator_manager->set_function(id,MOTOR_FORWARD)) {
        LOGE("motor%d set_function error\n", id);
    }

    sleep(10);
    if(vibrator_manager->set_function(id,MOTOR_BRAKE)) {
        LOGE("motor%d set_function error\n", id);
    }

    sleep(1);
    vibrator_manager->close(id);
    vibrator_manager->deinit();
    netlink_manager->stop();
    netlink_manager->deinit();
    return 0;
}
