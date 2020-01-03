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
#include <uart/uart_manager.h>

#define LOG_TAG "test_uart"

#define PORT_S1_DEVNAME             "/dev/ttyS2"
#define PORT_S1_BAUDRATE            3000000
#define PORT_S1_DATABITS            8
#define PORT_S1_PARITY              UART_PARITY_NONE
#define PORT_S1_STOPBITS            1
#define PORT_S1_TRANSMIT_TIMEOUT    3000  //ms
#define PORT_S1_TRANSMIT_LENGTH     10

int main(int argc, char *argv[]) {

    struct uart_manager* uart = get_uart_manager();
    int int_ret;
    char write_buf[] = "This is write test...\n\r";
    char read_buf[2048];

    if (uart == NULL){
        LOGE("fatal error on getting single instance uart\n\r");
        return -1;
    }

    int_ret = (int)uart->init(PORT_S1_DEVNAME, PORT_S1_BAUDRATE,
            PORT_S1_DATABITS, PORT_S1_PARITY, PORT_S1_STOPBITS);
    if (int_ret < 0) {
        LOGE("uart init called failed\n\r");
        return -1;
    }

#if 0
    /* This is not essential,  flow control is disabled on default */
    int_ret = uart->flow_control(PORT_S1_DEVNAME, UART_FLOWCONTROL_NONE);
    if (int_ret < 0) {
        LOGE("uart flow control called failed\n");
        return -1;
    }
#endif

    for (int i = 0; i < 32; i++) {
        int_ret = uart->write(PORT_S1_DEVNAME,
                write_buf, sizeof(write_buf), PORT_S1_TRANSMIT_TIMEOUT);
        if ((int_ret < 0) || (int_ret < sizeof(write_buf))) {
            LOGE("uart write called failed\n\r");
            return -1;
        }
    }
    LOGI("waiting for read ....\n\r");
    while(1) {
        int_ret = uart->read(PORT_S1_DEVNAME,
            read_buf, PORT_S1_TRANSMIT_LENGTH, PORT_S1_TRANSMIT_TIMEOUT);
        if ((int_ret < 0) || (int_ret < PORT_S1_TRANSMIT_LENGTH)){
            LOGE("uart read called failed\n\r");
            return -1;
        }

        LOGI("gotten: %s\n\r", read_buf);
    }
    uart->deinit(PORT_S1_DEVNAME);
    return 0;
}
