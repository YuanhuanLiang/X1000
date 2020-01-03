#include <assert.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include "spi/spi_manager.h"


extern void (*spi_wrreg)(uint8_t addr, uint8_t value);
extern uint8_t (*spi_rdreg)( uint8_t addr );

static struct spi_manager *spi;

void m_spi_wrreg(uint8_t addr, uint8_t value)
{
    int res;
    uint8_t txbuf[2];
    addr = ( ( addr & 0x3f ) << 1 ) & 0x7F; //code the first byte

    txbuf[0] = addr;
    txbuf[1] = value;
    res = spi->transfer(SPI_DEV0, txbuf, 2, NULL, 0);
    assert(res == 0);
    //printf("W [%02x]=[%02x]\n", (addr & 0x7f) >> 1, value);
}

uint8_t m_spi_rdreg( uint8_t addr )
{
    int res;
    uint8_t txbuf[1], rxbuf[1];

    addr = ( ( addr & 0x3f ) << 1 ) | 0x80; //code the first byte
    txbuf[0] = addr;
    res = spi->transfer(SPI_DEV0, txbuf, 1, rxbuf, 1);
    //printf("R [%02x]=[%02x]\n", (addr & 0x7f) >> 1, rxbuf[0]);
    assert(res == 0);

    return rxbuf[0];
}

void spi_init(enum spi id)
{
    uint32_t speed;
    uint8_t mode;
    uint8_t bits;
    static bool inited = false;

    assert(inited == false);
    inited = true;
    spi = get_spi_manager();

    //printf("SL2523_SPI_DEV: %s\n", SL2523_SPI_DEV);
    mode = SPI_MODE_0;
    bits = 8;
    speed = 10000000;
    spi->init(id, mode, bits, speed);

    spi_wrreg = m_spi_wrreg;
    spi_rdreg = m_spi_rdreg;
}
