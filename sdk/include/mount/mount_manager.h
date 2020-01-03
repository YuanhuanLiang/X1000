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

#include <utils/list.h>

struct mounted_volume {
    char device[64];
    char mount_point[64];
    char filesystem[64];
    char flags[128];
    struct list_head head;
};

struct mount_manager {
    void (*dump_mounted_volumes)(void);
    struct mounted_volume* (*find_mounted_volume_by_device)(const char* device);
    struct mounted_volume* (*find_mounted_volume_by_mount_point)(const char* mount_point);
    int (*umount_volume)(struct mounted_volume* volume);
    int (*mount_volume)(const char* device, const char* mount_point);
};

struct mount_manager* get_mount_manager(void);
