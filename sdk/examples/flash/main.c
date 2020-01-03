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
#include <flash/flash_manager.h>

#define LOG_TAG "test_flash"

#define TEST_FLASH_AREA_START           0x608000
#define TEST_FLASH_AREA_END              0x1000000
#define TEST_FLASH_AREA_LENGTH        (TEST_FLASH_AREA_END - 0x608000)

void generate_test_data(char *buf, int size) {
    for (int i = 0 ; i < size; i++) {
        buf[i] = i;
    }
}

int main(int argc, char *argv[]) {

    struct flash_manager* flash = get_flash_manager();
    int32_t ret;
    int64_t int64_ret;
    int64_t start_addr = TEST_FLASH_AREA_START;
    int64_t total_length = TEST_FLASH_AREA_LENGTH;
    int64_t offset, length;
    int32_t erase_unit = 0;
    int32_t io_unit = 0;
    if (flash == NULL){
        LOGE("fatal error on getting single instance flash\n");
        return -1;
    }

    char *write_buf = NULL;
    char *read_buf = NULL;
    int seed = 0;
    while (1) {
        LOGI("====running init ====\n");
        ret = flash->init();
        if (ret < 0) {
            LOGE("Flash init failed\n");
            return -1;
        }
        erase_unit = flash->get_erase_unit();
        LOGI("erase unit %d\n", erase_unit);
    #if 0
        io_unit = flash->get_io_unit();
        io_unit *= 8;
        LOGI("io unit %d\n", io_unit);
    #else
        io_unit = 128;
    #endif
        io_unit += seed;
        write_buf = malloc(io_unit);
        if (write_buf == NULL) {
            LOGE("Cannot alloc more memory\n");
            return -1;
        }
        read_buf = malloc(io_unit);
        if (read_buf == NULL) {
            LOGE("Cannot alloc more memory\n");
            goto out;
        }

        LOGI("====erasing at address 0x%llx with length 0x%llx ====\n",
                start_addr, total_length);
        int64_ret = flash->erase(start_addr, total_length);
        if (int64_ret < 0) {
            LOGE("erase failed at address 0x%llx with length 0x%llx\n", start_addr, total_length);
            goto out;
        }

        offset = TEST_FLASH_AREA_START;
        length = TEST_FLASH_AREA_LENGTH;
        while(1) {

            generate_test_data(write_buf, io_unit);

            if ((offset + io_unit) >= 0x1000000) {
                break;
            }
            LOGI("====writing at address 0x%llx with length %d ====\n",
                offset, io_unit);
            int64_ret = flash->write(offset, write_buf, io_unit);
            if (int64_ret < 0) {
                LOGE("write failed at offset 0x%llx with length %d\n", offset, io_unit);
                goto out;
            }
            LOGI("====reading at address 0x%llx with length %d ====\n",
                offset, io_unit);
            int64_ret = flash->read(offset, read_buf, io_unit);
            if (int64_ret < 0) {
                LOGE("read failed at offset 0x%llx with length %d\n", offset, io_unit);
                goto out;
            }

            if (memcmp(write_buf, read_buf, io_unit)) {
                LOGE("data transfer compare wrong at offset 0x%llx with length %d\n",
                        offset, io_unit);
                goto out;
            }
            memset(write_buf, 0, io_unit);
            memset(read_buf, 0, io_unit);
            offset += io_unit;
            length -= io_unit;
            if (length <= 0) {
                LOGI("====test done===\n");
                break;
            }
        }

        if (flash->deinit() < 0) {
            LOGE("Flash release failed\n");
            return -1;
        }
        seed++;
        if (seed >= 1024)
            seed = 0;
    }
    return 0;
out:
    if (flash->deinit() < 0) {
        LOGE("Flash release failed\n");
    }
    if (write_buf) {
        free(write_buf);
        write_buf = NULL;
    }
    if (read_buf) {
        free(read_buf);
        read_buf = NULL;
    }
    return -1;
}
