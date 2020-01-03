#ifndef MTD_H
#define MTD_H

#include <flash/block/fs/fs_manager.h>
#include <flash/block/block_manager.h>
// #define MTD_OPEN_DEBUG
#define MTD_CHAR_HEAD "/dev/mtd"
/* normal io unit size is page aligned,  this macro will close the hard restrict */
#define MTD_IO_SIZE_ARBITRARY

enum {
    MTD_BLK_PREFIX = 0x5A5A5A00,
    MTD_BLK_SCAN,
    MTD_BLK_BAD,
    MTD_BLK_ERASED,
    MTD_BLK_WRITEN,
};

struct mtd_block_map {
    char *name;
    unsigned int *es;
    int64_t eb_start;
    int64_t eb_cnt;
    int64_t bad_cnt;
    int write_eio;
};

struct mtd_part_char {
    char *path;
    int fd;
};

void set_process_info(struct filesystem *fs, int type, int64_t eboff, int64_t ebcnt);
int mtd_bm_block_map_set(struct filesystem *fs, int64_t eb, int status);
int mtd_get_blocksize_by_offset(struct block_manager* this, int64_t offset);
int mtd_type_is_nand(struct mtd_dev_info *mtd);
int mtd_type_is_mlc_nand(struct mtd_dev_info *mtd);
int mtd_type_is_nor(struct mtd_dev_info *mtd);
int mtd_is_block_scaned(struct block_manager* this, int64_t offset);
void mtd_scan_dump(struct filesystem *fs);
int64_t mtd_block_scan(struct filesystem *fs);
int mtd_basic_chiperase_preset(struct filesystem *fs);
void mtd_bm_block_map_destroy(struct block_manager *bm) ;
int64_t mtd_basic_erase(struct filesystem *fs);
int64_t mtd_basic_write(struct filesystem *fs);
int64_t mtd_basic_read(struct filesystem *fs);
struct mtd_dev_info* mtd_get_dev_info_by_offset(struct block_manager* this,
        int64_t offset);

#define MTD_DEV_INFO_TO_FD(mtd)   container_of(mtd, struct bm_part_info, part.mtd_dev_info)->fd
#define MTD_DEV_INFO_TO_START(mtd)  container_of(mtd, struct bm_part_info, part.mtd_dev_info)->start
#define MTD_DEV_INFO_TO_ID(mtd)     container_of(mtd, struct bm_part_info, part.mtd_dev_info)->id
#define MTD_DEV_INFO_TO_PATH(mtd)     container_of(mtd, struct bm_part_info, part.mtd_dev_info)->path
#define MTD_OFFSET_TO_EB_INDEX(mtd, off)   ((off)/mtd->eb_size)
#define MTD_IS_BLOCK_ALIGNED(mtd, off)  (((off)&(~mtd->eb_size + 1)) == (off))
#define MTD_BLOCK_ALIGN(mtd, off)   ((off)&(~mtd->eb_size + 1))
#define MTD_EB_RELATIVE_TO_ABSOLUTE(mtd, eb)    ((eb)+MTD_OFFSET_TO_EB_INDEX( \
                                                        mtd, MTD_DEV_INFO_TO_START(mtd)))
#define MTD_EB_ABSOLUTE_TO_RELATIVE(mtd, eb)    ((eb)-MTD_OFFSET_TO_EB_INDEX( \
                                                        mtd, MTD_DEV_INFO_TO_START(mtd)))
#endif
