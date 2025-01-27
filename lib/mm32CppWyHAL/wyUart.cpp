#include "wyUart.hpp"
#include "wyGpio.hpp"
#include "wySys.hpp"
#include "cppHalReg.hpp"

using namespace UART;

Serial *__uartX_objPtr[__UART_TotalNum];

Serial::Serial(uint8_t n, const char *tx, const char *rx, uint32_t baud)
{
    this->num = --n;
    this->uart = (UART_TypeDef *)__UART_BASEs[n];

    uint16_t const *rxAf = __UART_Rx_GPIO_AFs[n];
    uint16_t const *txAf = __UART_Tx_GPIO_AFs[n];
    uint8_t af;
    uint32_t clk;
    uint32_t rcc_enr;
    uint32_t rcc_mask;

    __uartX_objPtr[n] = this;
    af = ((__UART_BUS_CFG >> (n << 1)) & 0x03);
    rcc_enr = __UART_RCC_ENR_CFG_OFFSETs[n] >> 8;
    rcc_enr += RCC_BASE;
    rcc_mask = __UART_RCC_ENR_CFG_OFFSETs[n] & 0xff;
    rcc_mask = 0x01 << rcc_mask;

    *((uint32_t *)rcc_enr) |= rcc_mask;

    if (af == sys::__mcuBus::_MCU_BUS_APB1)
    {
        // RCC->APB1ENR |= __UART_RCC_EN[n];
        clk = sys::GetPCLK1Freq();
    }
    else if (af == sys::__mcuBus::_MCU_BUS_APB2)
    {
#ifdef __MM_Chip_Has_APB2
        // RCC->APB2ENR |= __UART_RCC_EN[n];
        clk = sys::GetPCLK2Freq();
#endif
    }
    af = 0xff;
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

#define __UART_WTF 0

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

enum __uartIrqMode
{
    __uartIrqMode_User = 0,
    __uartIrqMode_Fifo,
    __uartIrqMode_Cmd
};

void Serial::iqrHandler()
{
#if __UART_WTF
    if (this->uart->ISR & 0x02)
    {
        uint32_t d;
        this->uart->ICR = 0x02;
        d = this->uart->RDR;
        if (this->irqMode == __uartIrqMode_User)
        {
            this->IRQ_Callback(d);
            return;
        }
        if (this->irqMode == __uartIrqMode_Fifo)
        {
            this->fifo.byteProcess(d);
            return;
        }
        if (this->irqMode == __uartIrqMode_Cmd)
        {
            this->cmd.byteProcess(d);
            return;
        }
    }
#endif
}

inline void Serial::nvicCfg()
{
    this->uart->IER |= 0x02;
    NVIC_SetPriority(__UART_IRQ[this->num], 1);
    NVIC_EnableIRQ(__UART_IRQ[this->num]);
}

#if !__UART_WTF
void (*__uartRxIRQ_Callbacks[__UART_TotalNum])(uint8_t) = {nullptr, nullptr};
void *__uartADT_ptr[__UART_TotalNum] = {nullptr, nullptr};

template <uint32_t uartIdx, typename T>
void __uartRxIRQ_Lib(uint8_t d)
{
    ((T *)(__uartADT_ptr[uartIdx]))->byteProcess(d);
}

using _cmd = __wyIstream::CMD_Listener;
using _fifo = __wyIstream::FIFO;
void (*__uartRxIRQ_CMD_Listener[__UART_TotalNum])(uint8_t) = {
    __uartRxIRQ_Lib<0, _cmd>, __uartRxIRQ_Lib<1, _cmd>
#if __UART_TotalNum > 2
    ,
    __uartRxIRQ_Lib<2, _cmd>
#endif
#if __UART_TotalNum > 3
    ,
    __uartRxIRQ_Lib<3, _cmd>
#endif
};

void (*__uartRxIRQ_FIFO[__UART_TotalNum])(uint8_t) = {
    __uartRxIRQ_Lib<0, _fifo>, __uartRxIRQ_Lib<1, _fifo>
#if __UART_TotalNum > 2
    ,
    __uartRxIRQ_Lib<2, _fifo>
#endif
#if __UART_TotalNum > 3
    ,
    __uartRxIRQ_Lib<3, _fifo>
#endif
};
#endif

void Serial::interruptCMD(bool s)
{
    if (s)
    {
        this->uart->IER |= 0x0002;
        NVIC_EnableIRQ(__UART_IRQ[this->num]);
        return;
    }
    this->uart->IER &= ~0x0002;
    NVIC_DisableIRQ(__UART_IRQ[this->num]);
}
void Serial::setInterrupt(uint8_t *buf, uint32_t bufSize, char const *start, char const *end, void (*f)(uint8_t *, uint32_t))
{
    this->cmd.setBuf(buf, bufSize);
    // this->setTrigger(start, end);
    this->cmd.setKeyWord(start, end);
    if (f != nullptr)
        this->addCMD(f);
#if __UART_WTF
    this->irqMode = __uartIrqMode_Cmd;
#else
    __uartADT_ptr[this->num] = &(this->cmd);
    __uartRxIRQ_Callbacks[this->num] = __uartRxIRQ_CMD_Listener[this->num];
#endif
    this->nvicCfg();
}
void Serial::setInterrupt(uint8_t *buf, uint32_t bufSize, char const *start, char const *end, uint8_t *argvBuf, uint32_t argvBufLen, uint32_t *argcPtr)
{
    this->cmd.setBuf(buf, bufSize);
    // this->setTrigger(start, end);
    this->cmd.setKeyWord(start, end);
    this->cmd.setArgvArgcBuff(argvBuf, argvBufLen, argcPtr);
#if __UART_WTF
    this->irqMode = __uartIrqMode_Cmd;
#else
    __uartADT_ptr[this->num] = &(this->cmd);
    __uartRxIRQ_Callbacks[this->num] = __uartRxIRQ_CMD_Listener[this->num];
#endif
    this->nvicCfg();
}
void Serial::setInterrupt(uint8_t *buf, uint32_t bufSize)
{
    this->nvicCfg();
    this->fifo.setFifoBuf(buf, bufSize);
#if __UART_WTF
    this->irqMode = __uartIrqMode_Fifo;
#else
    __uartADT_ptr[this->num] = &(this->fifo);
    __uartRxIRQ_Callbacks[this->num] = __uartRxIRQ_FIFO[this->num];
#endif
}

void Serial::setInterrupt(void (*f)(uint8_t))
{
    this->nvicCfg();
#if __UART_WTF
    this->IRQ_Callback = f;
    this->irqMode = __uartIrqMode_User;
#else
    __uartRxIRQ_Callbacks[this->num] = f;
#endif
}

#if __UART_WTF
#define __UARTx_IRQHandler(__UARTx) __uartX_objPtr[__UARTx - 1]->iqrHandler()
#else
#define __UARTx_IRQHandler(__UARTx)                        \
    if (UART##__UARTx->ISR & 0x02)                         \
    {                                                      \
        uint32_t rd;                                       \
        UART##__UARTx->ICR = 0x02;                         \
        rd = UART##__UARTx->RDR;                           \
        if (__uartRxIRQ_Callbacks[__UARTx - 1] != nullptr) \
            __uartRxIRQ_Callbacks[__UARTx - 1](rd & 0xff); \
    }
#endif

extern "C"
{
    void UART1_IRQHandler(void) { __UARTx_IRQHandler(1); }
    void UART2_IRQHandler(void) { __UARTx_IRQHandler(2); }
#if __UART_TotalNum > 3
    void UART3_4_IRQHandler(void)
    {
        __UARTx_IRQHandler(4);
        __UARTx_IRQHandler(3);
    }
#endif
}
