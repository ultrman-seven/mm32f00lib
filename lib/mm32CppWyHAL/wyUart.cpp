#include "wyUart.hpp"
using namespace UART;

// #include "queue"
// #include "list"
#include "wyGpio.hpp"
#include "wySys.hpp"

// std::queue<uint8_t, std::list<uint8_t>> __uartFifo[2];

const uint16_t __UART1_Rx_GPIO_AFs[] = {__GPIO_AF_Val(0, 0, 1), __GPIO_AF_Val(0, 3, 1), __GPIO_AF_Val(0, 13, 1), 0xffff};
const uint16_t __UART1_Tx_GPIO_AFs[] = {__GPIO_AF_Val(0, 12, 1), __GPIO_AF_Val(0, 14, 1), 0xffff};
const uint16_t __UART2_Rx_GPIO_AFs[] = {__GPIO_AF_Val(0, 13, 2), 0xffff};
const uint16_t __UART2_Tx_GPIO_AFs[] = {__GPIO_AF_Val(0, 1, 2), 0xffff};

Serial::Serial(uint8_t n, const char *tx, const char *rx, uint32_t baud)
{
    this->num = --n;
    this->uart = (UART_TypeDef *)__UART_BASEs[n];
    __RCC_UART_ENR |= __UART_RCC_EN[n];

    uint16_t const *rxAf = n ? __UART2_Rx_GPIO_AFs : __UART1_Rx_GPIO_AFs;
    uint16_t const *txAf = n ? __UART2_Tx_GPIO_AFs : __UART1_Tx_GPIO_AFs;
    uint8_t af = 0xff;
    uint32_t clk = sys::GetPCLK1Freq();

    if (rx != nullptr)
    {
        af = GPIO::afTable2afVal(rx, rxAf);
        if (af != 0xff)
        {
            GPIO::afConfig(rx, af, GPIO::Mode_IN_FLOATING);
            this->uart->GCR |= 0x08;
        }
    }
    if (tx != nullptr)
    {
        af = GPIO::afTable2afVal(tx, txAf);
        if (af != 0xff)
        {
            GPIO::afConfig(tx, af, GPIO::Mode_AF_PP);
            // GPIO::afConfig("a1", 2, GPIO::Mode_AF_PP);
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
    // std::queue<uint8_t, std::list<uint8_t>> empty;
    // __uartFifo[num].swap(empty);
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
#include "stdio.h"
Serial &Serial::operator<<(uint8_t dat)
{
    sendByte(dat);
    return *this;
}

Serial &Serial::operator<<(const char dat)
{
    sendByte(dat);
    return *this;
}
Serial &Serial::operator<<(int32_t num)
{
    char str[10];
    sprintf(str, "%d", num);
    this->operator<<(str);
    return *this;
}
Serial &Serial::operator<<(float num)
{
    char str[10];
    sprintf(str, "%f", num);
    this->operator<<(str);
    return *this;
}
Serial &Serial::operator<<(const char *s)
{
    while (*s)
        sendByte(*s++);
    return *this;
}
Serial &Serial::operator<<(char *s)
{
    while (*s)
        sendByte(*s++);
    return *this;
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

// Serial &Serial::operator>>(uint8_t &dat)
// {
//     dat = this->receiveByte();
//     return *this;
// }

// void (*__uartRxIRQ_Callbacks[2])(uint8_t) = {nullptr, nullptr};

#define __UARTx_IRQHandler(__UARTx)                                        \
    if (UART##__UARTx->ISR & 0x02)                                         \
    {                                                                      \
        UART##__UARTx->ICR = 0x02;                                         \
        if (__uartRxIRQ_Callbacks[__UARTx - 1] == nullptr)                 \
            /* __uartFifo[__UARTx - 1].push(UART##__UARTx->RDR & 0x0f); */ \
            __uartFifo[__UARTx - 1].emplace(UART##__UARTx->RDR & 0x0f);    \
        else                                                               \
            __uartRxIRQ_Callbacks[__UARTx - 1](UART##__UARTx->RDR & 0x0f); \
    }

// void UART1_IRQHandler(void) { __UARTx_IRQHandler(1); }
// void UART2_IRQHandler(void) { __UARTx_IRQHandler(2); }
