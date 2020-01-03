/*
 *  测试方法:
 *   windows准备工作
 *      打开设备管理器 -> 人体学输入设备 -> 查询是否存在vid=0525 pid=a4ac的设备
 *      如果上述设备存在，任意打开一个记事本文件，等待linux端的数据输入
 *  在linux终端下执行以下命令
 *      启动测试程序
 *           /root/export/test_usb_hid /dev/hidg0 keyboard
 *
 *          开始测试, 请仔细观察输入每条命令序列后的现象
 *          g i s t r --left-shift
 *          --return
 *          g i s t r --hold
 *          g i s t r a
 *          --return
 *          --caps-lock
 *          a b c d
 *          注意: 单次输入的最大命令长度为6个字节，每个命令之间用空格分割
 */
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <usb/usb_manager.h>

/* 传输超时时间 */
#define TEST_HID_TRANSFER_TIMEOUT    100

#define BUF_LEN 512

struct options {
    const char    *opt;
    unsigned char val;
};

static struct options kmod[] = {
    {.opt = "--left-ctrl",      .val = 0x01},
    {.opt = "--right-ctrl",     .val = 0x10},
    {.opt = "--left-shift",     .val = 0x02},
    {.opt = "--right-shift",    .val = 0x20},
    {.opt = "--left-alt",       .val = 0x04},
    {.opt = "--right-alt",      .val = 0x40},
    {.opt = "--left-meta",      .val = 0x08},
    {.opt = "--right-meta",     .val = 0x80},
    {.opt = NULL}
};

static struct options kval[] = {
    {.opt = "--return", .val = 0x28},
    {.opt = "--esc",    .val = 0x29},
    {.opt = "--bckspc", .val = 0x2a},
    {.opt = "--tab",    .val = 0x2b},
    {.opt = "--spacebar",   .val = 0x2c},
    {.opt = "--caps-lock",  .val = 0x39},
    {.opt = "--f1",     .val = 0x3a},
    {.opt = "--f2",     .val = 0x3b},
    {.opt = "--f3",     .val = 0x3c},
    {.opt = "--f4",     .val = 0x3d},
    {.opt = "--f5",     .val = 0x3e},
    {.opt = "--f6",     .val = 0x3f},
    {.opt = "--f7",     .val = 0x40},
    {.opt = "--f8",     .val = 0x41},
    {.opt = "--f9",     .val = 0x42},
    {.opt = "--f10",    .val = 0x43},
    {.opt = "--f11",    .val = 0x44},
    {.opt = "--f12",    .val = 0x45},
    {.opt = "--insert", .val = 0x49},
    {.opt = "--home",   .val = 0x4a},
    {.opt = "--pageup", .val = 0x4b},
    {.opt = "--del",    .val = 0x4c},
    {.opt = "--end",    .val = 0x4d},
    {.opt = "--pagedown",   .val = 0x4e},
    {.opt = "--right",  .val = 0x4f},
    {.opt = "--left",   .val = 0x50},
    {.opt = "--down",   .val = 0x51},
    {.opt = "--kp-enter",   .val = 0x58},
    {.opt = "--up",     .val = 0x52},
    {.opt = "--num-lock",   .val = 0x53},
    {.opt = NULL}
};

int keyboard_fill_report(char report[8], char buf[BUF_LEN], int *hold)
{
    char *tok = strtok(buf, " ");
    int key = 0;
    int i = 0;

    for (; tok != NULL; tok = strtok(NULL, " ")) {

        if (strcmp(tok, "--quit") == 0)
            return -1;

        if (strcmp(tok, "--hold") == 0) {
            *hold = 1;
            continue;
        }

        if (key < 6) {
            for (i = 0; kval[i].opt != NULL; i++)
                if (strcmp(tok, kval[i].opt) == 0) {
                    report[2 + key++] = kval[i].val;
                    break;
                }
            if (kval[i].opt != NULL)
                continue;
        }

        if (key < 6)
            if (islower(tok[0])) {
                report[2 + key++] = (tok[0] - ('a' - 0x04));
                continue;
            }

        for (i = 0; kmod[i].opt != NULL; i++)
            if (strcmp(tok, kmod[i].opt) == 0) {
                report[0] = report[0] | kmod[i].val;
                break;
            }
        if (kmod[i].opt != NULL)
            continue;

        if (key < 6)
            fprintf(stderr, "unknown option: %s\n", tok);

        printf("dumping report: ");
        for (i = 0; i < 8; i++) {
            printf("0x%x ", report[i]);
        }
        printf("\n");
    }
    return 8;
}

void print_options(char c)
{
    int i = 0;

    if (c == 'k') {
        printf("    keyboard options:\n"
               "        --hold\n");
        for (i = 0; kmod[i].opt != NULL; i++)
            printf("\t\t%s\n", kmod[i].opt);
        printf("\n  keyboard values:\n"
               "        [a-z] or\n");
        for (i = 0; kval[i].opt != NULL; i++)
            printf("\t\t%-8s%s", kval[i].opt, i % 2 ? "\n" : "");
        printf("\n");
    }
}

int main(int argc, const char *argv[])
{
    char *filename = NULL;
    char buf[BUF_LEN];
    int cmd_len;
    char report[8];
    int to_send = 0;
    int hold = 0;
    fd_set rfds;
    int i;
    int retval;

    struct usb_device_manager *hid = NULL;
    unsigned int max_pkt_size = 0;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s devname keyboard\n",
            argv[0]);
        return -1;
    }

    if (argv[2][0] != 'k')
      return -1;

    filename = (char*)argv[1];

    hid = get_usb_device_manager();
    retval = hid->init(filename);
    if (retval < 0) {
        printf("HID init failed \n");
        return -1;
    }

    max_pkt_size = hid->get_max_transfer_unit(filename);

    print_options(argv[2][0]);
    while (42) {

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, NULL);
        if (retval == -1 && errno == EINTR)
            continue;
        if (retval < 0) {
            perror("select()");
            return 4;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            /*从终端读取键盘输入的数据*/
            cmd_len = read(STDIN_FILENO, buf, BUF_LEN - 1);

            if (cmd_len == 0)
                break;

            buf[cmd_len - 1] = '\0';
            hold = 0;

            /*填充hid report数据*/
            memset(report, 0x0, sizeof(report));
            if (argv[2][0] == 'k')
                to_send = keyboard_fill_report(report, buf, &hold);

            if (max_pkt_size != to_send) {
                printf("Cannot support %d size to be send, the max packet size is %d\n",
                        to_send, max_pkt_size);
                goto out;
            }
            /*发送hid report数据*/
            if (hid->write(filename, report, to_send, TEST_HID_TRANSFER_TIMEOUT) != to_send) {
                printf("write faild\n");
                goto out;
            }
            /*是否关闭了重复发送功能*/
            if (!hold) {
                memset(report, 0x0, sizeof(report));
                if (hid->write(filename, report, to_send, TEST_HID_TRANSFER_TIMEOUT) != to_send) {
                    printf("write failed\n");
                    goto out;
                }
            }
            /*若有反馈数据，则读取并打印*/
            if ((retval = hid->read(filename, buf, max_pkt_size, TEST_HID_TRANSFER_TIMEOUT)) < 0)  {
                printf("read faild\n");
                goto out;
            }
            if (retval == 0) {
                /* 当超时时间设置为0时，该情况有可能发生 */
                continue;
            }
            printf("retcode: \n");
            for (i = 0; i < retval; i++) {
                printf("0x%x ", buf[i]);
            }
            printf("\n");
        }
    }
    return 0;
out:
    hid->deinit(filename);
    return -1;
}
