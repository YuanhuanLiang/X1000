/*************************************************************************
    > Filename: main.c
    >   Author: Wang Qiuwei
    >    Email: qiuwei.wang@ingenic.com / panddio@163.com
    > Datatime: Fri 04 May 2018 04:07:26 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/spi/spidev.h>

#include <types.h>
#include <utils/log.h>
#include <gsm_manager.h>
#include <mh1902_manager.h>

#define LOG_TAG    "mpos_app"

void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("CTRL+C has been keydown\n");
        mh1902_spi_deinit();
        mh1902_gpio_deinit();
        gsm_uart_deinit();
        gsm_gpio_deinit();
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    unsigned char txbuf[6] = {0x00, 0x0f, 0xf0, 0x66, 0x5a, 0x3c};
    unsigned char rxbuf[1024];
    int i, retval;
    int count = 0;

    signal(SIGINT, sig_handler);

    retval= mh1902_gpio_init();
    if (retval < 0) {
        LOGE("Failed to call mh1902_gpio_init()\n");
        return retval;
    }
    retval = mh1902_spi_init(SPI_MODE_0, 8, 2000000);
    if (retval < 0) {
        LOGE("Failed to call mh1902_spi_init()\n");
        return retval;
    }

    retval = gsm_gpio_init();
    if (retval < 0) {
        LOGE("Failed to call gsm_gpio_init()\n");
        return retval;
    }
    retval = gsm_uart_init();
    if (retval < 0) {
        LOGE("Failed to call gsm_uart_init()\n");
        gsm_gpio_deinit();
        return retval;
    }

    printf("======= Test %s module ========\n", GSM_MODULE_NAME);
    gsm_power_on();

#if 0
    gsm_uart_write("ATE0\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);
#endif

    do {
        gsm_uart_write("AT+CPIN?\r\n");
        sleep(1);
        bzero(rxbuf, sizeof(rxbuf));
        gsm_uart_read(rxbuf, sizeof(rxbuf));
        LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);
        if (strstr(rxbuf, "+CPIN: READY"))
            break;
    } while(1);

    sleep(8);

    gsm_uart_write("ATI8\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);

    gsm_uart_write("AT+CSQ\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);

    gsm_uart_write("AT+CREG?\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);

    gsm_uart_write("AT+CGREG?\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);

    gsm_uart_write("AT+CGATT?\r\n");
    bzero(rxbuf, sizeof(rxbuf));
    gsm_uart_read(rxbuf, sizeof(rxbuf));
    LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);

    if (argc > 1 && !strcmp(argv[1], "-f")) {
        unsigned char cmd[128];
        while(1) {
            bzero(cmd, sizeof(cmd));
            printf("> ");
            fflush(stdout);
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strlen(cmd) - 1] = '\r';
            cmd[strlen(cmd)] = '\n';
            cmd[strlen(cmd) + 1] = '\0';

            if (strcmp(cmd, "q\r\n") == 0)
                break;
            else if (strlen(cmd) == 2)
                continue;

            gsm_uart_write(cmd);
            bzero(rxbuf, sizeof(rxbuf));
            gsm_uart_read(rxbuf, sizeof(rxbuf));
            printf("rxstr=%s\n", rxbuf);
        }
    }

#if 0
    printf("======= Test MH1902 SPI =========\n");
    retval = mh1902_spi_transfer(txbuf, rxbuf, sizeof(txbuf));
    if (retval < 0) {
        printf("ERROR: spi transfer failed\n");
        goto exit;
    }

    for (i = 0; i < sizeof(txbuf); i++)
        printf("rxbuf[%d]=0x%02x\n", i, rxbuf[i]);
#endif

    printf("======== ms800_app exit =========\n");

exit:
    mh1902_spi_deinit();
    mh1902_gpio_deinit();
    gsm_uart_deinit();
    gsm_gpio_deinit();

    return 0;
}
