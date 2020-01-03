#ifndef SYSINFO_MANAGER_H
#define SYSINFO_MANAGER_H

enum sysinfo_id {
    SYSINFO_RESERVED,   //0x3c00: flash parameter   0x3c6c:  partition info
};

struct sysinfo_layout {
    int64_t offset;
    int64_t length;
    char *value;
};

enum sysinfo_operation {
    SYSINFO_OPERATION_RAM,
    SYSINFO_OPERATION_DEV,
};

#define SYSINFO_RESERVED_OFFSET  0x3c00
#define SYSINFO_RESERVED_SIZE    0x400

struct sysinfo_manager {
    int64_t (*get_offset)(struct sysinfo_manager *this, int id);
    int64_t (*get_length)(struct sysinfo_manager *this, int id);
    int (*get_value)(struct sysinfo_manager *this, int id, char *buf, char flag);
    int (*traversal_save)(struct sysinfo_manager *this, int64_t offset, int64_t length);
    int (*traversal_merge)(struct sysinfo_manager *this, char *buf, int64_t offset, int64_t length);
    int (*init)(struct sysinfo_manager *this);
    int (*exit)(struct sysinfo_manager *this);
    void *binder;
};

void sysinfo_manager_bind(struct sysinfo_manager *this, void *target);

extern struct sysinfo_manager sysinfo;

#define GET_SYSINFO_BINDER(t)   (t->binder)
#define GET_SYSINFO_ID(t)  (SYSINFO_##t)
#define GET_SYSINFO_MANAGER()  (&(sysinfo))

#endif