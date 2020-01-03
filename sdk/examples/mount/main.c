#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <utils/log.h>
#include <utils/common.h>
#include <utils/compare_string.h>
#include <mount/mount_manager.h>
#include <netlink/netlink_manager.h>

#define LOG_TAG "test_mount"

static const char* prefix_volume_mount_point = "/mnt";
static const char* prefix_volume_device_path = "/dev";

static struct mount_manager* mount_manager;
static struct netlink_manager* netlink_manager;
static struct netlink_handler* netlink_handler;

static void handle_block_event(struct netlink_handler* nh,
        struct netlink_event* event) {
    int error = 0;

    char dev_path[128];
    char mp_path[128];

    int nparts = -1;
    const char* type = event->find_param(event, "DEVTYPE");
    const char* name = event->find_param(event, "DEVNAME");
    const char* nparts_str = event->find_param(event, "NPARTS");
    const int action = event->get_action(event);

    memset(dev_path, 0, sizeof(dev_path));
    memset(mp_path, 0, sizeof(mp_path));

    if (nparts_str)
        nparts = atoi(nparts_str);

    if (!is_prefixed_with(name, "sd") && !is_prefixed_with(name, "mmcblk"))
        return;

    if (action == NLACTION_ADD) {
        if ((!strcmp(type, "disk") && !nparts) || (!strcmp(type, "partition"))) {

            sprintf(dev_path, "%s/%s", prefix_volume_device_path, name);
            sprintf(mp_path, "%s/%s", prefix_volume_mount_point, name);

            error = mount_manager->mount_volume(dev_path, mp_path);
            if (error < 0)
                LOGE("Faied to mount device \"%s\"\n", dev_path);
        }

    } else if (action == NLACTION_REMOVE) {
        if ((!strcmp(type, "disk") && !nparts) || (!strcmp(type, "partition"))) {

            sprintf(dev_path, "%s/%s", prefix_volume_device_path, name);
            struct mounted_volume *volume = mount_manager->find_mounted_volume_by_device(dev_path);
            if (volume == NULL)
                return;

            mount_manager->umount_volume(volume);
            if (error < 0)
                LOGE("Failed to umount device \"%s\": %s\n", volume->device,
                        strerror(errno));
        }
    }
}

static void handle_event(struct netlink_handler* nh,
        struct netlink_event* event) {
    const char* subsystem = event->get_subsystem(event);

    if (!strcmp(subsystem, "block"))
        event->dump(event);

    if (!strcmp(subsystem, "block"))
        handle_block_event(nh, event);
}

int main(int argc, char *argv[]) {
    int error = 0;

    mount_manager = get_mount_manager();
    if (mount_manager == NULL) {
        LOGE("Failed to get mount manager.\n");
        return -1;
    }

    netlink_handler = (struct netlink_handler *) calloc(1, sizeof(struct netlink_handler));
    netlink_handler->construct = construct_netlink_handler;
    netlink_handler->deconstruct = destruct_netlink_handler;
    netlink_handler->construct(netlink_handler, "all sub-system", 0, handle_event, NULL);

    netlink_manager = get_netlink_manager();
    if (netlink_manager == NULL) {
        LOGE("Failed to get netlink manager.\n");
        return -1;
    }

    error = netlink_manager->init();
    if (error) {
        LOGE("Failed to init netlink manager.\n");
        return -1;
    }

    netlink_manager->register_handler(netlink_handler);

    error = netlink_manager->start();
    if (error) {
        LOGE("Failed to start netlink manager.\n");
        return -1;
    }

    cold_boot("/sys/class/block");

    while (1)
        sleep(1000);

    netlink_handler->deconstruct(netlink_handler);
    free(netlink_handler);

    return 0;
}
