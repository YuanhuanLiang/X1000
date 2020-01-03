#include <unistd.h>

#include <utils/log.h>
#include <utils/common.h>
#include <battery/battery_manager.h>
#include <netlink/netlink_manager.h>
#include <power/power_manager.h>

#define LOG_TAG "test_battery"

static struct battery_manager* battery_manager;
static struct netlink_manager* netlink_manager;
static struct power_manager* power_manager;

static void battery_event_listener(struct battery_event* event) {
    battery_manager->dump_event(event);

    if (event->capacity == 0 && event->state != POWER_SUPPLY_STATUS_CHARGING)
        power_manager->power_off();
}

int main(int argc, char *argv[]) {
    int error = 0;

    battery_manager = get_battery_manager();
    if (battery_manager == NULL) {
        LOGE("Failed to get battery manager.\n");
        return -1;
    }

    netlink_manager = get_netlink_manager();
    if (netlink_manager == NULL) {
        LOGE("Failed to get netlink manager.\n");
        return -1;
    }

    power_manager = get_power_manager();
    if (power_manager == NULL) {
        LOGE("Failed to get power manager.\n");
        return -1;
    }

    error = battery_manager->init();
    if (error < 0) {
        LOGE("Failed to init battery manager.\n");
        return -1;
    }

    battery_manager->register_event_listener(battery_event_listener);

    error = netlink_manager->init();
    if (error) {
        LOGE("Failed to init netlink manager.\n");
        return -1;
    }

    netlink_manager->register_handler(battery_manager->get_netlink_handler());

    error = netlink_manager->start();
    if (error) {
        LOGE("Failed to start netlink manager.\n");
        return -1;
    }

    cold_boot("/sys/class/power_supply");

    while (1)
        sleep(1000);

    return 0;
}
