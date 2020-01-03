/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Wed 06 Jun 2018 03:57:50 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>

#include <qrcode/tuteng_decode.h>

#define LOG_TAG    "test_ttdecode"

static uint32 uiDecodeCnt = 0;

/**
 * decode_get_data_cb - 解码后的回调函数
 * @pBuf: 解码结果缓存区指针
 * @uibuflen: 解码结果的长度
 */
void decode_get_data_cb(uint8 *pBuf, uint32 uiBufLen)
{
    printf("decode data[%04d]:%s, Len=%d\r\n",
            ++uiDecodeCnt, pBuf, uiBufLen);
}

/**
 * sig_handler - 键盘按 CTRL+C 的信号处理函数
 *@signo: 信号值
 */
void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("CTRL+C has been keydown\n");
        cim_decode_stop();
        cim_decode_deinit();
        exit(0);
    }
}

static void usage(FILE *fp)
{
    fprintf(fp,
            "Entry cmd:\n"
            "0      LCD dose not display image\n"
            "1      LCD disable image\n"
            "s      start to decode\n"
            "S      stop decode\n"
            "e      exit\n"
            "\n");
}

int main(int argc, char *argv[])
{
    printf("libttdecode ver:%s\r\n", cim_decode_get_version());

    /* 初始化，显示图像 */
    cim_decode_init(EN_DISPLAY);

    /* 注册解码后的回调函数 */
    cim_decode_get_data_register(decode_get_data_cb);

    /* 开始抓图解码 */
    cim_decode_start();

    while(1) {
        switch(getchar()) {
        case '0':
            cim_decode_init(NO_DISPLAY);
            break;
        case '1':
            cim_decode_init(EN_DISPLAY);
            break;
        case 's':
            cim_decode_start();
            break;
        case 'S':
            cim_decode_stop();
            break;
        case 'e':
            goto exit;
            break;
        default:
            usage(stdout);
        }
    }

exit:
    /* 停止解码 */
    cim_decode_stop();
    cim_decode_deinit();
    return 0;
}
