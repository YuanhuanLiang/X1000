/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Wed 12 Jul 2017 11:44:25 AM CST
 ************************************************************************/

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

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>
#include <input/input_manager.h>
#include <cypress/cypress_manager.h>


#define LOG_TAG    "cypress_testunit"

struct input_manager *input;

/*
 * Functions
 */
static void keys_report_handler(uint8_t keycode, uint8_t keyvalue)
{
    printf("Cypress touch-keys, keycode: %d, state: %s\n",
            keycode, keyvalue ? "pressed" : "released");
}

static void cypress_input_event_listener(const char *input_name,
        struct input_event *event) {
    if (event->type == EV_KEY) {
        keys_report_handler(event->code, event->value);
    }
}

static void card_report_handler(int dev_fd)
{
    uint8_t i;
    struct card_t card;

    bzero(&card, sizeof(struct card_t));

    /**
     * 获取卡的UID
     */
    if (ioctl(dev_fd, CYPRESS_DRV_IOC_R_CARD_UID, &card) < 0) {
        LOGE("Failed to read card UID\n");
        return;
    }

    for (i = 0; i < 4; i++) {
        printf("uid[%d]=0x%x\n", i, card.uid[i]);
    }

    /**
     * 判断卡的类型
     */
    if (card.cardtype == CARD_MIFARE1) {
        LOGI("cardtype: CARD_MIFARE1\n");

#if 1
        /**
         * 读M1的某个扇区的流程，目前只支持一次读一个扇区
         */
        card.sector = 0;
        card.keytype = M1_AUTH_KEYA;
        card.m1keyA[0] = 0xff;
        card.m1keyA[1] = 0xff;
        card.m1keyA[2] = 0xff;
        card.m1keyA[3] = 0xff;
        card.m1keyA[4] = 0xff;
        card.m1keyA[5] = 0xff;

        if (ioctl(dev_fd, CYPRESS_DRV_IOC_R_CARD_SECTOR, &card) > 0) {
            for (i = 0; i < 64; i++) {
                if(i%16==0) {
                    printf("\nblock%d:", i/16);
                }
                printf("0x%02x  ", card.data[i]);
            }
            printf("\n");
            fflush(stdout);
        } else {
            /**
             * Failed to read card sector
             */
        }
#endif
        /**
         * 不管是否有读M1卡扇区的操作，都必须发送ACK
         */
        ioctl(dev_fd, CYPRESS_DRV_IOC_SEND_ACK, NULL);
    }
}

int main(int argc, char *argv[])
{
    struct cypress_manager *cypress;

    cypress = get_cypress_manager();
    if (cypress->init(card_report_handler) < 0) {
        LOGE("Failed to init cypress manager\n");
        return -1;
    }

    input = get_input_manager();
    if (input->init() < 0) {
        LOGE("Failed to init input manager\n");
        return -1;
    }

    input->register_event_listener(CYPRESS_INPUT_DEV_NAME, cypress_input_event_listener);
    if (input->start() < 0) {
        LOGE("Failed to start input manager\n");
        return -1;
    }

    while(1) {
        usleep(100);
    };

    cypress->deinit();
    input->stop();
    input->unregister_event_listener(CYPRESS_INPUT_DEV_NAME, cypress_input_event_listener);
    input->deinit();
    return 0;
}
