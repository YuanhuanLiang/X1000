#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "sl2523.h"
#include "Rfid_interface.h"
#include "spi/spi_manager.h"

extern void sl2523_gpio_init(unsigned int io_res, unsigned int io_irq);
extern void spi_init(enum spi id);

#define GPIO_PA(n)      (32*0 + n)
#define GPIO_PB(n)      (32*1 + n)
#define GPIO_PC(n)      (32*2 + n)
#define GPIO_PD(n)      (32*3 + n)
#define GPIO_PE(n)      (32*4 + n)

unsigned int SL2523_GPIO_INT = GPIO_PC(17);
unsigned int SL2523_GPIO_PD = GPIO_PC(18);

int main(int argc, char **argv)
{
    int ret = 0;
    spi_init(SPI_DEV0);
    sl2523_gpio_init(SL2523_GPIO_PD, SL2523_GPIO_INT);

    T_CardInfo info;

    while (1) {
        ret = detech_card_ab(&info);
        if (1 == ret) {
            uint32_t uid;
            memcpy(&uid, info.aCardInfo.serial_num, 4);
            printf("the card ID: 0x%08X\n", uid);

        } else if (2 == ret) {
            for (int i = 0; i < 10; i++) {
                    printf("%4d", info.bCardInfo.serial_num[i]);
                }
        }
        usleep(1000*100);
    }

    return 0;
}
