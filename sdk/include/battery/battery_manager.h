/*
 *  Copyright (C) 2017, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <types.h>

enum {
        POWER_SUPPLY_STATUS_UNKNOWN = 0,
        POWER_SUPPLY_STATUS_CHARGING,
        POWER_SUPPLY_STATUS_DISCHARGING,
        POWER_SUPPLY_STATUS_NOT_CHARGING,
        POWER_SUPPLY_STATUS_FULL,
};

enum {
    POWER_SUPPLY_TECHNOLOGY_UNKNOWN = 0,
    POWER_SUPPLY_TECHNOLOGY_NiMH,
    POWER_SUPPLY_TECHNOLOGY_LION,
    POWER_SUPPLY_TECHNOLOGY_LIPO,
    POWER_SUPPLY_TECHNOLOGY_LiFe,
    POWER_SUPPLY_TECHNOLOGY_NiCd,
    POWER_SUPPLY_TECHNOLOGY_LiMn,
};

enum {
    POWER_SUPPLY_HEALTH_UNKNOWN = 0,
    POWER_SUPPLY_HEALTH_GOOD,
    POWER_SUPPLY_HEALTH_OVERHEAT,
    POWER_SUPPLY_HEALTH_DEAD,
    POWER_SUPPLY_HEALTH_OVERVOLTAGE,
    POWER_SUPPLY_HEALTH_UNSPEC_FAILURE,
    POWER_SUPPLY_HEALTH_COLD,
    POWER_SUPPLY_HEALTH_WATCHDOG_TIMER_EXPIRE,
    POWER_SUPPLY_HEALTH_SAFETY_TIMER_EXPIRE,
};

struct battery_event {
    uint8_t state;
    uint32_t capacity;
    uint32_t voltage_now;
    uint32_t voltage_max;
    uint32_t voltage_min;
    uint32_t present;
    uint32_t technology;
    uint32_t health;
};

typedef void (*battery_event_listener_t)(struct battery_event *event);

struct battery_manager {
    int (*init)(void);
    int (*deinit)(void);
    void (*register_event_listener)(battery_event_listener_t listener);
    void (*unregister_event_listener)(battery_event_listener_t listener);
    struct netlink_handler* (*get_netlink_handler)(void);
    void (*dump_event)(struct battery_event* event);
};

struct battery_manager* get_battery_manager(void);

#endif /* BATTERY_MANAGER_H */
