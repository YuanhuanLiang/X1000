#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <security/security_manager.h>

#define LOG_TAG "test_security"

//#define AES_CBC

uint32_t test_aes_key[4] = {
        0x2b7e1516, 0x28aed2a6, 0xabf71588,0x09cf4f3c,
};

uint32_t test_indata[4] = {
        0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734,
};

#ifdef AES_CBC
uint32_t test_iv[4] = {
        0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
};
#endif

uint32_t test_buf[4];
uint32_t test_outdata[4];

struct aes_key aes_key;
struct aes_data aes_data;

int main(int argc, char* argv[]) {
    int error = 0;
    struct security_manager *security_manager;

    security_manager = get_security_manager();

    aes_key.key = (uint8_t*) test_aes_key;
    aes_key.keylen = sizeof(test_aes_key);

#ifdef AES_CBC
    aes_key.iv = (uint8_t*) test_iv;
    aes_key.ivlen = sizeof(test_iv);
    aes_key.aesmode = AES_MODE_CBC;
#else
    aes_key.aesmode = AES_MODE_ECB;
#endif

    aes_key.bitmode = AES_KEY_128BIT;

    if (security_manager->init() < 0) {
        LOGE("Failed init security manager\n");
        return -1;
    }

    error = security_manager->simple_aes_load_key(&aes_key);
    if (error < 0) {
        LOGE("Failed to load aes key\n");
        return -1;
    }

    LOGI("=============================\n");
    LOGI("Encrypt:\n");
    for (int i = 0; i < sizeof(test_indata) / 4; i++)
        LOGI("input data[%d]=0x%x\n", i, test_indata[i]);

    aes_data.encrypt = 0;
    aes_data.input = (uint8_t*) test_indata;
    aes_data.output = (uint8_t*) test_buf;
    aes_data.input_len = sizeof(test_indata);
    error = security_manager->simple_aes_crypt(&aes_data);
    if (error < 0) {
        LOGE("Failed to encrypt\n");
        return -1;
    }

    LOGI("\n");
    for (int i = 0; i < sizeof(test_buf) / 4; i++)
        LOGI("encrypt data[%d]=0x%x\n", i, test_buf[i]);
    LOGI("=============================\n");

    aes_data.encrypt = 1;
    aes_data.input = (uint8_t*) test_buf;
    aes_data.output = (uint8_t*) test_outdata;
    aes_data.input_len = sizeof(test_buf);
    error = security_manager->simple_aes_crypt(&aes_data);
    if (error < 0) {
        LOGE("Failed to decrypt\n");
        return -1;
    }

    if (memcmp(test_indata, test_outdata, sizeof(test_indata))) {
        LOGE("Decrypt error\n");
        return -1;
    }

    LOGI("=============================\n");
    LOGI("Decrypt:\n");
    for (int i = 0; i < sizeof(test_outdata) / 4; i++)
        LOGI("out data[%d]=0x%x\n", i, test_outdata[i]);
    LOGI("=============================\n");

    security_manager->deinit();

    return 0;
}
