#ifndef JFFS2_H
#define JFFS2_H
#include <flash/block/fs/fs_manager.h>
#include <linux/jffs2.h>
int jffs2_init_cleanmarker(struct filesystem *fs,
                         struct jffs2_unknown_node *maker,
                         int *pos, int *len);
int jffs2_write_cleanmarker(struct filesystem *fs,
                           int64_t offset,
                           struct jffs2_unknown_node *cleanmarker,
                           int clmpos, int clmlen);
#endif
