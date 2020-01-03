/*
 *  Copyright (C) 2016, Zhang YanMing <jamincheung@126.com>
 *
 *  Linux recovery updater
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
#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <inttypes.h>
#include <lib/mtd/libmtd.h>
#include <types.h>

#define BM_BLOCK_TYPE_MTD   "mtd"

#define BM_FILE_TYPE_NORMAL  "normal"

// #define BM_SYSINFO_SUPPORT

#define BM_MTD_FILE_TYPE_INIT(name)     \
    char *name[] = {                    \
        BM_FILE_TYPE_NORMAL,      \
    }

enum bm_operation {
    BM_OPERATION_ERASE = 0x100,
    BM_OPERATION_WRITE,
    BM_OPERATION_READ,
    BM_OPERATION_ERASE_WRITE,
    BM_OPERATION_FORMAT,
};

enum bm_operation_method {
    BM_OPERATION_METHOD_PARTITION = 0x200,
    BM_OPERATION_METHOD_RANDOM,
};

#define BM_OPERATE_METHOD_INIT(name)    \
    uint32_t method[] = {               \
        BM_OPERATION_METHOD_PARTITION,  \
        BM_OPERATION_METHOD_RANDOM,     \
    }

struct bm_operate_prepare_info {
    pid_t tid;
    int64_t write_start;
    uint32_t physical_unit_size;
    uint32_t logical_unit_size;
    int64_t max_size_mapped_in_partition;
    void *context_handle;
};

struct bm_operation_option {
    int method;         /* one in block_operation_method*/
    char filetype[20];  /* one in BM_FILE_TYPE_INIT*/
};

struct bm_event {
    char *part_name;
    int operation;
    int progress;
};

struct block_manager;

typedef void (*bm_event_listener_t)(struct block_manager *bm,
        struct bm_event* event, void* param);

union bm_dev_info {
    struct mtd_dev_info mtd_dev_info;
};
struct bm_part_info {
    union bm_dev_info part;
    int64_t start;
    int fd;
    char path[16];
    int id;
};

struct bm_mtd_info {
    libmtd_t mtd_desc;
    struct mtd_info mtd_info;
    void *map;
    bm_event_listener_t event_listener;
};

union bm_info {
    struct bm_mtd_info mtd;
};

struct block_manager {
    void (*dump_event)(struct block_manager* this, struct bm_event* event);
    void (*get_supported)(struct block_manager* this, char *buf);
    void (*get_supported_filetype)(struct block_manager* this, char *buf);

    int (*set_operation_option)(struct block_manager* this, struct bm_operation_option* opion,
                                int method, char *filetype);
    int64_t (*erase)(struct block_manager* this, int64_t offset,
                     int64_t length);
    int64_t (*write)(struct block_manager* this, int64_t offset,
                     char* buf, int64_t length);
    int64_t (*read)(struct block_manager* this, int64_t offset, char* buf,
                    int64_t length);
    uint32_t (*get_prepare_io_size)(struct block_manager* this);
    uint32_t (*get_prepare_leb_size)(struct block_manager* this);
    int64_t (*get_prepare_write_start)(struct block_manager* this);
    int64_t (*get_prepare_max_mapped_size)(struct block_manager* this);
    int64_t (*finish)(struct block_manager* this);

    int64_t (*get_partition_size_by_offset)(struct block_manager* this,
                                            int64_t offset);
    int64_t (*get_partition_size_by_name)(struct block_manager* this,
                                          char *name);
    int64_t (*get_partition_start_by_offset)(struct block_manager* this,
            int64_t offset);
    int64_t (*get_partition_start_by_name)(struct block_manager* this,
                                           char *name);
    int (*get_partition_count)(struct block_manager* this);
    int64_t (*get_capacity)(struct block_manager* this);
    int (*get_blocksize)(struct block_manager* this, int64_t offset);
    int (*get_iosize)(struct block_manager* this, int64_t offset);
    struct bm_operation_option operate_option;
    struct bm_operate_prepare_info *prepared;
    union bm_info desc;
    struct bm_part_info *part_info;
#ifdef BM_SYSINFO_SUPPORT
    struct sysinfo_manager *sysinfo;
#endif
    char *name;
    struct list_head  list_cell;
    struct list_head  list_fs_head;
    void* param;
};

#define BM_GET_MTD_DESC(bm) ((bm->desc.mtd.mtd_desc))
#define BM_GET_MTD_INFO(bm) (&(bm->desc.mtd.mtd_info))
#define BM_GET_MTD_BLOCK_MAP(bm, type) ((type**)(&bm->desc.mtd.map))
#define BM_GET_LISTENER(bm) (bm->desc.mtd.event_listener)
#define BM_GET_PARTINFO(bm) (bm->part_info)
#define BM_GET_PARTINFO_START(bm, i) (bm->part_info[i].start)
#define BM_GET_PARTINFO_FD(bm, i) (&(bm->part_info[i].fd))
#define BM_GET_PARTINFO_PATH(bm, i) (bm->part_info[i].path)
#define BM_GET_PARTINFO_ID(bm, i) (bm->part_info[i].id)
#define BM_GET_PARTINFO_MTD_DEV(bm, i)  (&(bm->part_info[i].part.mtd_dev_info))
#define BM_GET_PREPARE_INFO(bm)   (bm->prepared)
#define BM_GET_PREPARE_INFO_CONTEXT(bm) (bm->prepared->context_handle)

struct block_manager * mtd_manager_init(void);
int mtd_manager_destroy(struct block_manager *m);

#endif /* block_manager_H */
