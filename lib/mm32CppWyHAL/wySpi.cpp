#include "wySpi.hpp"

using namespace SPI;

uint8_t SpiObject::readWrite(uint8_t dat)
{
    uint32_t ret;
    // uint32_t sr;
    spi->TDR = dat;

    while (!(spi->SR & 1))
        ;
    while (!(spi->SR & 2))
        ;
    ret = spi->RDR;
    return ret & 0x00ff;
}

void SpiObject::readWrite(uint8_t *txBuf, uint8_t *rxBuf, uint32_t len)
{
    uint32_t ret[4];
    // while (len--)
    // {
        // spi->TDR = *txBuf++;
        // spi->TDR = 0;

        // while (!(spi->SR & 1))
        //     ;
        // while (!(spi->SR & 2))
        //     ;
        ret[0] = spi->RDR;
        // spi->TDR = 0;
        ret[1] = spi->RDR;
        // spi->TDR = 0;
        ret[2] = spi->RDR;
        // spi->TDR = 0;
        ret[3] = spi->RDR;
        // (*rxBuf++) = ((spi->RDR) & 0x00ff);
        rxBuf[3] = (ret[0] & 0x00ff);
        rxBuf[2] = (ret[1] & 0x00ff);
        rxBuf[1] = (ret[2] & 0x00ff);
        rxBuf[0] = (ret[3] & 0x00ff);
    // }
}

#include "reg_rcc.h"
SpiObject::SpiObject(uint8_t num, const char *sclk, const char *mosi, const char *miso)
{
    // clock 4 spi
    RCC->APB1ENR |= RCC_APB1ENR_SPI1;
    spi = SPI1;

    // gpio af
    if (nullptr != sclk)
    {
        GPIO::afConfig(sclk, 0, GPIO::Mode_AF_PP);
    }
    if (nullptr != mosi)
    {
        GPIO::afConfig(mosi, 0, GPIO::Mode_AF_PP);
    }
    if (nullptr != miso)
    {
        GPIO::afConfig(miso, 0, GPIO::Mode_IN_FLOATING);
        // spi->GCR |= 0x10;
    }

    // spi cfg
    spi->GCR &= ~((0x01U << (11)));
    // spi->GCR; // nss,mode
    spi->CCR |= 1; // lsb,cpol,cpha
    spi->CCR |= 2;
    spi->CCR |= ((0x01U << (3)));

    spi->BRR = 8;
    // spi->BRR = 14;

    spi->EXTCTL = 8;

    // rx tx enable
    (spi->GCR) |= ((0x01U << (4)));
    (spi->GCR) |= ((0x01U << (3)));
    // spi enable
    spi->GCR |= 1;
}
