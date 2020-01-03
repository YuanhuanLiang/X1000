/*
 *  测试方法:
 *  在linux终端下执行以下命令
 *      启动测试程序
 *          /root/export/test_usb_cdc_acm /dev/ttyGS0
 *
 *  windows下准备工作
 *      安装驱动
 *          安装linux-cdc-acm.inf驱动
 *              驱动配置文件位置: “工程顶级目录”/kernel/Documentation/usb/linux-cdc-acm.inf
 *          安装后可以识别到串口设备，默认的vid:pid为0525:a4a7
 *          开启串口终端，指定任意波特率/起始位/停止位
 *  重新回到linux测试终端
 *          在终端输入以下字符，仔细观察windows下的串口终端现象
 *              abcd+回车
 *  在windows查看测试结果
 *          在windows终端下查看是否收到abcd,若收到，马上[本测试程序有5s的超时读等待时间]输入dcba回车, 并切换回linux端查看现象
 *          若没有收到dcba,请查看连接是否异常
 *          注意: 由于单次读取数据大小设置为5个字节， 因此测试时终端发送的字节数应该大于等于5， 否则会阻塞读直到超时
 *
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
#include <utils/common.h>
#include <thread/thread.h>

/* 传输超时时间 */
#define TEST_CDC_ACM_WRITE_TIMEOUT    100
#define TEST_CDC_ACM_READ_TIMEOUT     5000
#define TEST_READ_SIZE                               5
#define BUF_LEN 512

static struct usb_device_manager *cdc_acm = NULL;
static char *filename = NULL;
static char buf[BUF_LEN];
static int max_read_size = TEST_READ_SIZE;

static void thread_loop(struct pthread_wrapper* thread, void* param) {
    int retval = 0;

    for (;;) {
        /*若有反馈数据，则读取并打印*/
        printf("waiting %d miliseconds for read\n", TEST_CDC_ACM_READ_TIMEOUT);
        if ((retval = cdc_acm->read(filename, buf, max_read_size, TEST_CDC_ACM_READ_TIMEOUT)) < 0)  {
            printf("read faild\n");
            continue;
        }
        if (retval == 0) {
            /* 当超时时间设置为0时，该情况有可能发生 */
            printf("read timeout has occured\n");
            continue;
        }
        printf("received byte counts: %d\n", retval);
        for (int i = 0; i < retval; i++) {
            printf("0x%x ", buf[i]);
        }
        printf("\n");
    }
}

int main(int argc, const char *argv[])
{
    int cmd_len;
    int to_send = 0;
    fd_set rfds;
    int retval;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s devname\n",
            argv[0]);
        return -1;
    }

    struct thread *runner = _new(struct thread, thread);
    runner->runnable.run = thread_loop;

    cdc_acm = get_usb_device_manager();
    filename = (char*)argv[1];

    retval = cdc_acm->init(filename);
    if (retval < 0) {
        printf("USB cdc acm init failed \n");
        return -1;
    }

     /* 通信测试 */
    printf("read unit size is %d\n", max_read_size);
    runner->start(runner, NULL);

    while (1) {

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

            /*发送从键盘输入的数据*/
            to_send = strlen(buf) + 1;
            if (cdc_acm->write(filename, buf, to_send, TEST_CDC_ACM_WRITE_TIMEOUT) != to_send) {
                printf("write faild\n");
                goto out;
            }
        }
    }

    return 0;
out:
    cdc_acm->deinit(filename);
    return -1;
}
