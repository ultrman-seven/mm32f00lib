#include "wySpi.hpp"

using namespace SPI;

#define SpiCrtState_TxEmpty 0x01
#define SpiCrtState_Rx 0x02
#define SpiCrtState_TxFull 0x04
#define SpiCrtState_Rx4Byte 0x08

#define SpiIntState_TxFifoEmpty 0x01
#define SpiIntState_RxOk 0x02
#define SpiIntState_RxOver 0x08
#define SpiIntState_RxMatch 0x10
#define SpiIntState_RxFull 0x20
#define SpiIntState_TxCplt 0x40

#define SpiGenCtrl_Enable 0x01
#define SpiGenCtrl_Disable 0x00

#define SpiGenCtrl_IntEnable 0x02
#define SpiGenCtrl_IntDisable 0x00

#define SpiGenCtrl_ModeMaster 0x04
#define SpiGenCtrl_ModeSlave 0x00

#define SpiGenCtrl_TxEn 0x08
#define SpiGenCtrl_RxEn 0x10

#define SpiGenCtrl_NssEn 0x0400
#define SpiGenCtrl_NssToggleEn 0x1000

#define SpiGenCtrl_DataWidth_8bit 0x00
#define SpiGenCtrl_DataWidth_32bit 0x0800

#define SpiComCtrl_ClkPhaseEdge1 0x0001
#define SpiComCtrl_ClkPhaseEdge2 0x0000

#define SpiComCtrl_ClkPolarityIdleHigh 0x0002
#define SpiComCtrl_ClkPolarityIdleLow 0x0000

#define SpiComCtrl_Lsb 0x0004
#define SpiComCtrl_Msb 0x0000

#define SpiComCtrl_RxEdgeTail 0x0010
#define SpiComCtrl_TxNo1ClkDelay 0x0020

static SpiObject *_obj[1];

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

void SpiObject::readWriteAsync(uint8_t *txBuf, uint8_t *rxBuf, uint32_t len, void (*cbk)(void *), void *arg)
{
    this->cbk = cbk;
    this->arg = arg;

    this->txBuf = txBuf;
    this->rxBuf = rxBuf;
    this->txLen = len;
    this->rxLen = len;

    this->spi->GCTL |= (SpiGenCtrl_RxEn | SpiGenCtrl_TxEn | SpiGenCtrl_IntEnable);
    this->spi->INTEN |= (SpiIntState_RxOk | SpiIntState_TxFifoEmpty);
    this->spi->GCTL |= SpiGenCtrl_Enable;
}

void SpiObject::readAsync(uint8_t *buf, uint32_t len, void (*cbk)(void *), void *arg)
{
    this->cbk = cbk;
    this->arg = arg;

    this->rxBuf = buf;
    this->txBuf = nullptr;
    this->txLen = 0;
    this->rxLen = len;

    this->spi->GCTL &= ((uint32_t)(~(SpiGenCtrl_TxEn)));
    this->spi->GCTL |= (SpiGenCtrl_RxEn | SpiGenCtrl_IntEnable);
    this->spi->RXDNR = 1;
    this->spi->INTEN |= SpiIntState_RxOk;
    this->spi->GCTL |= SpiGenCtrl_Enable;
}

void SpiObject::writeAsync(uint8_t *buf, uint32_t len, void (*cbk)(void *), void *arg)
{
    this->cbk = cbk;
    this->arg = arg;

    this->txBuf = buf;
    this->rxBuf = nullptr;
    this->txLen = len;
    this->rxLen = 0;

    this->spi->GCTL &= ((uint32_t)(~(SpiGenCtrl_RxEn)));
    this->spi->GCTL |= (SpiGenCtrl_TxEn);

    this->spi->INTEN |= (SpiIntState_TxCplt | SpiIntState_TxFifoEmpty);
    this->spi->GCTL |= SpiGenCtrl_Enable;
    this->spi->GCTL |= SpiGenCtrl_IntEnable;
}

#include "reg_rcc.h"
SpiObject::SpiObject(uint8_t num, const char *sclk, const char *mosi, const char *miso, GPIO::Mode mosiMode, uint16_t clkDiv, uint8_t clkCfg)
{
    // clock 4 spi
    RCC->APB1ENR |= RCC_APB1ENR_SPI1;
    spi = SPI1;

    _obj[0] = this;

    // gpio af
    if (nullptr != sclk)
    {
        GPIO::afConfig(sclk, 0, GPIO::Mode_AF_PP);
    }
    if (nullptr != mosi)
    {
        GPIO::afConfig(mosi, 0, mosiMode);
    }
    if (nullptr != miso)
    {
        GPIO::afConfig(miso, 0, GPIO::Mode_IN_FLOATING);
    }

    spi->GCR &= (uint32_t)(~(SpiGenCtrl_DataWidth_32bit));

    spi->GCR |= SpiGenCtrl_ModeMaster;

    spi->CCR |= (clkCfg);
    spi->BRR = clkDiv;

    spi->EXTCTL = 8;
}

namespace SPI
{
    void spiIrqHandle(SpiObject *handle)
    {
        if ((handle->spi->INTSTAT) & SpiIntState_TxFifoEmpty)
        {
            handle->spi->TDR = *(handle->txBuf);
            --(handle->txLen);
            if (0 == (handle->txLen))
            {
                handle->spi->INTEN &= ((uint32_t)(~(SpiIntState_TxFifoEmpty)));
            }
        }

        if ((handle->spi->INTSTAT) & SpiIntState_TxCplt)
        {
            (handle->spi->INTCLR) = SpiIntState_TxCplt;
            if (0 == (handle->txLen))
            {
                handle->spi->INTEN &= ((uint32_t)(~(SpiIntState_TxCplt)));
                handle->spi->GCTL &= ((uint32_t)(~(SpiGenCtrl_Enable | SpiGenCtrl_RxEn | SpiGenCtrl_TxEn | SpiGenCtrl_IntEnable)));
                if (handle->cbk)
                    handle->cbk(handle->arg);
            }
        }

        if ((handle->spi->INTSTAT) & SpiIntState_RxOk)
        {
            (handle->spi->INTCLR) = SpiIntState_RxOk;
            *(handle->rxBuf) = (handle->spi->RDR);
            --(handle->rxLen);
            ++(handle->rxBuf);
            if (0 == (handle->rxLen))
            {
                handle->spi->GCTL &= ((uint32_t)(~(SpiGenCtrl_Enable | SpiGenCtrl_RxEn | SpiGenCtrl_IntEnable)));
                handle->spi->INTEN &= ((uint32_t)(~(SpiIntState_RxOk)));

                if (handle->cbk)
                    handle->cbk(handle->arg);
            }
        }
    }
} // namespace SPI

extern "C"
{
    void SPI1_IRQHandler(void)
    {
        spiIrqHandle(_obj[0]);
    }
}
