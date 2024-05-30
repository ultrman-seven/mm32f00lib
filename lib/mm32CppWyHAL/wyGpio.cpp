#include "wyGpio.hpp"
#include "reg_exti.h"
#include "reg_common.h"
#include "core_cm0.h"

using namespace GPIO;

static inline uint8_t __GPIO_PinName2PinData(const char *name, GPIO_TypeDef *&port, uint8_t &pinNum)
{
    char p = *name;

    p -= ((p >= 'a' && p <= 'z') ? 'a' : 'A');
    port = (GPIO_TypeDef *)__GPIO_PORT_BASEs[p];
    __RCC_GPIO_ENR |= __GPIO_PORT_RCC_EN[p];
    pinNum = 0;
    while (*++name)
    {
        pinNum *= 10;
        pinNum += *name - '0';
    }
    return p;
}

GpioPin::GpioPin(const char *n, Mode m, Speed s)
{
    GPIO_TypeDef *pt;
    uint8_t pn;
    __GPIO_PinName2PinData(n, pt, pn);
    this->port = pt;
    this->pinNum = pn;
    this->pin = 0x01 << pn;
    modeConfig(pt, pn, m, s);
}
void GpioPin::setMode(Mode m, Speed s)
{
    modeConfig(this->port, this->pinNum, m, s);
}

GpioPin &GpioPin::operator<<(bool s)
{
    if (s)
        this->port->BSRR |= this->pin;
    else
        this->port->BRR |= this->pin;
    return *this;
}
GpioPin &GpioPin::operator>>(bool &s)
{
    s = (this->port->IDR & this->pin);
    return *this;
}

void GpioPin::set(void) { this->port->BSRR = this->pin; }
void GpioPin::reset(void) { this->port->BRR = this->pin; }
void GpioPin::flip(void) { this->operator=(!((this->port->ODR) & (this->pin))); }
bool GpioPin::read(void) { return this->port->IDR & this->pin; }
// void GpioPin::operator=(bool s) { s ? (this->port->ODR |= this->pin) : (this->port->ODR &= (~this->pin)); }
void GpioPin::operator=(bool s) { s ? this->set() : this->reset(); }
void GpioPin::operator=(GPIO::GpioPin &o) { this->operator=(o.read()); }
bool GpioPin::operator!(void) { return !(this->port->IDR & this->pin); }

void (*__GPIO_EXTI_Callbacks[16])(void) = {nullptr};
#define __EXTI_LINE0 (0x0001)
#define __EXTI_LINE1 (0x0002)
#define __EXTI_LINE2 (0x0004)
#define __EXTI_LINE3 (0x0008)
#define __EXTI_LINE4 (0x0010)
#define __EXTI_LINE5 (0x0020)
#define __EXTI_LINE6 (0x0040)
#define __EXTI_LINE7 (0x0080)

#define __EXTI_LINE8 (0x0100)
#define __EXTI_LINE9 (0x0200)
#define __EXTI_LINE10 (0x0400)
#define __EXTI_LINE11 (0x0800)
#define __EXTI_LINE12 (0x1000)
#define __EXTI_LINE13 (0x2000)
#define __EXTI_LINE14 (0x4000)
#define __EXTI_LINE15 (0x8000)

#define __EXTIx_IRQHandler(__EXTIx)                        \
    if (EXTI->PR & __EXTI_LINE##__EXTIx)                   \
        if (EXTI->IMR & __EXTI_LINE##__EXTIx)              \
        {                                                  \
            EXTI->PR = __EXTI_LINE##__EXTIx;               \
            if (__GPIO_EXTI_Callbacks[__EXTIx] != nullptr) \
                __GPIO_EXTI_Callbacks[__EXTIx]();          \
        }

void EXTI0_1_IRQHandler(void)
{
    __EXTIx_IRQHandler(0);
    __EXTIx_IRQHandler(1);
}

void EXTI2_3_IRQHandler(void)
{
    __EXTIx_IRQHandler(2);
    __EXTIx_IRQHandler(3);
}

void EXTI4_15_IRQHandler(void)
{
    __EXTIx_IRQHandler(4);
    __EXTIx_IRQHandler(5);
    __EXTIx_IRQHandler(6);
    __EXTIx_IRQHandler(7);
    __EXTIx_IRQHandler(8);
    __EXTIx_IRQHandler(9);
    __EXTIx_IRQHandler(10);
    __EXTIx_IRQHandler(11);
    __EXTIx_IRQHandler(12);
    __EXTIx_IRQHandler(13);
    __EXTIx_IRQHandler(14);
    __EXTIx_IRQHandler(15);
}

namespace GPIO
{
    void modeConfig(const char *n, Mode m, Speed s)
    {
        GPIO_TypeDef *pt;
        uint8_t pn;
        __GPIO_PinName2PinData(n, pt, pn);
        modeConfig(pt, pn, m, s);
    }
    void modeConfig(GPIO_TypeDef *port, uint8_t pinNum, Mode m, Speed s)
    {
        uint32_t mask1, mask2;
        uint8_t pn = pinNum;
        s = Speed_10MHz;
        if (pn >= 8)
            pn -= 8;
        pn <<= 2;
        mask1 = (0x0f << pn);
        mask1 = ~mask1;
        // if bit 2 == 1 then mode is output. otherwise input.
        mask2 = (m & 0x04) ? s : 0;
        // get bit0 and bit1
        mask2 |= ((m & 0x03) << 2);
        mask2 <<= pn;

        if (pinNum >= 8)
        {
            port->CRH &= mask1;
            port->CRH |= mask2;
        }
        else
        {
            port->CRL &= mask1;
            port->CRL |= mask2;
        }

        if (m == Mode_IPU)
            port->BSRR = (0x01 << pinNum);
        if (m == Mode_IPD)
            port->BRR = (0x01 << pinNum);

        // get bit4 and bit5
        mask2 = m & 0x30;
        mask2 >>= 4;
        mask1 = 0x03;
        pn = pinNum;
        pn <<= 1;
        mask2 <<= pn;
        mask1 <<= pn;
        mask1 = ~mask1;
        port->DCR &= mask1;
        port->DCR |= mask2;
    }
    uint8_t afTable2afVal(char const *rx, uint16_t const *rxAf)
    {
        uint8_t port, pin, af = 0xff;
        port = *rx;
        port -= (port >= 'a' && port <= 'z') ? 'a' : 'A';
        pin = rx[1] - '0';
        if (rx[2])
            pin = pin * 10 + rx[2] - '0';
        while (*rxAf != 0xffff)
        {
            if (__GPIO_AF_Val(port, pin, 0) == (*rxAf & 0xff00))
            {
                af = *rxAf & 0x0f;
                return af;
            }
            ++rxAf;
        }
        return af;
    }
    void afConfig(const char *p, uint8_t af, Mode m)
    {
        GPIO_TypeDef *port;
        uint8_t pinNum;
        uint64_t mask1 = 0x0f;
        uint64_t mask2 = af & 0x0f;
        uint64_t *afr;

        __GPIO_PinName2PinData(p, port, pinNum);

        modeConfig(port, pinNum, m);
        // if (pinNum >= 8)
        //     pinNum -= 8;
        pinNum <<= 2;
        mask1 <<= pinNum;
        mask2 <<= pinNum;

        afr = (uint64_t *)&(port->AFRL);
        *afr &= ~mask1;
        *afr |= mask2;
    }

    void extiConfig(const char *n, void (*f)(void), Mode m)
    {
        GPIO_TypeDef *pt;
        uint8_t pn, tmp, portIdx;
        uint16_t line;
        uint32_t mask1, mask2;
        portIdx = __GPIO_PinName2PinData(n, pt, pn);
        modeConfig(pt, pn, m);
        __GPIO_EXTI_Callbacks[pn] = f;

        __EXTI_RCC_EN();
        line = 0x01 << pn;

        tmp = pn;
        tmp >>= 2;
        mask1 = 0x0f;
        mask2 = portIdx;

        portIdx = (pn%4)<<2;
        mask1 <<= portIdx;
        mask2 <<= portIdx;

        EXTI->CR[tmp] &= ~mask1;
        EXTI->CR[tmp] |= mask2;

        EXTI->IMR |= line;
        if (m == Mode_IPD)
            EXTI->RTSR |= line;
        else
            EXTI->FTSR |= line;

        // nvic
        IRQn_Type extiN;
        if (pn <= 1)
        {
            extiN = EXTI0_1_IRQn;
            mask1 = (uint32_t)EXTI0_1_IRQHandler;
        }
        else if (pn <= 3)
        {
            extiN = EXTI2_3_IRQn;
            mask1 = (uint32_t)EXTI2_3_IRQHandler;
        }
        else
        {
            extiN = EXTI4_15_IRQn;
            mask1 = (uint32_t)EXTI4_15_IRQHandler;
        }

        // NVIC_SetVector(extiN, mask1);
        NVIC_SetPriority(extiN, 1);
        NVIC_EnableIRQ(extiN);
        // exti cfg
    }
}
