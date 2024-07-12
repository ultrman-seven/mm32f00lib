#include "wyUart.hpp"
#include "wyGpio.hpp"
#include "wySys.hpp"
#include "cppHalReg.hpp"

using namespace UART;

Serial::Serial(uint8_t n, const char *tx, const char *rx, uint32_t baud)
{
    this->num = --n;
    this->uart = (UART_TypeDef *)__UART_BASEs[n];
    __RCC_UART_ENR |= __UART_RCC_EN[n];

    uint16_t const *rxAf = __UART_Rx_GPIO_AFs[n];
    uint16_t const *txAf = __UART_Tx_GPIO_AFs[n];
    uint8_t af = 0xff;
    uint32_t clk = sys::GetPCLK1Freq();

    if ((rx != nullptr) && (0 != *rx))
    {
        af = GPIO::afTable2afVal(rx, rxAf);
        if (af != 0xff)
        {
            // GPIO::afConfig(rx, af, GPIO::Mode_IN_FLOATING);
            GPIO::afConfig(rx, af, GPIO::Mode_IPU);
            this->uart->GCR |= 0x08;
        }
    }
    if ((tx != nullptr) && (0 != *tx))
    {
        af = GPIO::afTable2afVal(tx, txAf);
        if (af != 0xff)
        {
            GPIO::afConfig(tx, af, GPIO::Mode_AF_PP);
            this->uart->GCR |= 0x10;
        }
    }

    this->uart->CCR |= 0x30;
    this->uart->BRR = (clk / baud) / 16;
    this->uart->FRA = (clk / baud) % 16;
    this->uart->GCR |= 0x01;
}

Serial::~Serial()
{
}

// bool Serial::readBuff(uint8_t &d)
// {
//     if (__uartFifo[num].empty())
//         return false;
//     d = __uartFifo[num].front();
//     __uartFifo[num].pop();
//     return true;
// }
// bool Serial::readBuff(uint8_t *d, uint8_t &len)
// {
//     if (__uartFifo[num].empty())
//         return false;
//     len = 0;
//     while (!__uartFifo[num].empty())
//     {
//         *d++ = __uartFifo[num].front();
//         __uartFifo[num].pop();
//         ++len;
//     }
//     return true;
// }

// void Serial::putChar(uint8_t c)
// {
//     this->uart->TDR = (c & (uint16_t)0x00FF);
//     while (!(this->uart->CSR & 0x01))
//         ;
// }

// uint8_t Serial::receiveByte(void)
// {
//     return this->uart->RDR & (uint8_t)0x00fF;
// }

void Serial::putChar(uint8_t c)
{
    this->sendByte(c);
}

void Serial::sendByte(uint8_t dat)
{
    this->uart->TDR = (dat & (uint8_t)0xFF);
    while (!(this->uart->CSR & 0x01))
        ;
}

void Serial::sendByte(uint8_t *dat, uint8_t len)
{
    while (len--)
    {
        this->uart->TDR = (*dat++ & (uint8_t)0xFF);
        while (!(this->uart->CSR & 0x01))
            ;
    }
}

inline void Serial::nvicCfg()
{
    this->uart->IER |= 0x02;
    NVIC_SetPriority(__UART_IRQ[this->num], 1);
    NVIC_EnableIRQ(__UART_IRQ[this->num]);
}

#define __UART_TotalNum 2
void (*__uartRxIRQ_Callbacks[__UART_TotalNum])(uint8_t) = {nullptr, nullptr};
void *__uartADT_ptr[__UART_TotalNum] = {nullptr, nullptr};

template <uint32_t uartIdx, typename T>
void __uartRxIRQ_Lib(uint8_t d)
{
    ((T *)(__uartADT_ptr[uartIdx]))->byteProcess(d);
}

using _cmd = __wyIstream::CMD_Listener;
using _fifo = __wyIstream::FIFO;
void (*__uartRxIRQ_CMD_Listener[__UART_TotalNum])(uint8_t) = {__uartRxIRQ_Lib<0, _cmd>, __uartRxIRQ_Lib<1, _cmd>};
void (*__uartRxIRQ_FIFO[__UART_TotalNum])(uint8_t) = {__uartRxIRQ_Lib<0, _fifo>, __uartRxIRQ_Lib<1, _fifo>};

void Serial::setInterrupt(uint8_t *buf, uint32_t bufSize, char const *start, char const *end)
{
    this->cmd.setBuf(buf, bufSize);
    // this->setTrigger(start, end);
    this->cmd.setKeyWord(start, end);
    __uartADT_ptr[this->num] = &(this->cmd);
    __uartRxIRQ_Callbacks[this->num] = __uartRxIRQ_CMD_Listener[this->num];
    this->nvicCfg();
}
void Serial::setInterrupt(uint8_t *buf, uint32_t bufSize)
{
    this->nvicCfg();
    __uartADT_ptr[this->num] = &(this->fifo);
    __uartRxIRQ_Callbacks[this->num] = __uartRxIRQ_FIFO[this->num];
}
void Serial::setInterrupt(void (*f)(uint8_t))
{
    this->nvicCfg();
    __uartRxIRQ_Callbacks[this->num] = f;
}

#define __UARTx_IRQHandler(__UARTx)                        \
    uint32_t rd;                                           \
    if (UART##__UARTx->ISR & 0x02)                         \
    {                                                      \
        UART##__UARTx->ICR = 0x02;                         \
        rd = UART##__UARTx->RDR;                           \
        if (__uartRxIRQ_Callbacks[__UARTx - 1] != nullptr) \
            __uartRxIRQ_Callbacks[__UARTx - 1](rd & 0xff); \
    }

extern "C"
{
    void UART1_IRQHandler(void) { __UARTx_IRQHandler(1); }
    void UART2_IRQHandler(void) { __UARTx_IRQHandler(2); }
}
