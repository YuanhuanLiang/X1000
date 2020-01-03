#ifndef __JZ_UART_H__
#define __JZ_UART_H__

#include <soc/gpio.h>

struct uart_wakeup_pin {
    int num;
    unsigned long trigger_edge;
    enum gpio_function def_func;
};

struct jz_uart_platform_data {
    struct uart_wakeup_pin *wakeup_pin;
};

#endif
