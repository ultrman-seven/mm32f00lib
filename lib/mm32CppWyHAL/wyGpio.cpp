#include "wyGpio.hpp"
#include "wySys.hpp"

#include "reg_exti.h"
#include "reg_common.h"
#include "core_cm0.h"
#include "cppHalReg.hpp"

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
    this->reInit(n, m, s);
}

void GpioPin::reInit(const char *n, Mode m, Speed s)
{
    GPIO_TypeDef *pt;
    uint8_t pn;
    this->currentMode = m;
    if (n == nullptr || *n == 0)
    {
        this->port = nullptr;
        return;
    }
    this->portNum = __GPIO_PinName2PinData(n, pt, pn);
    this->port = pt;
    this->pinNum = pn;
    this->pin = 0x01 << pn;
    modeConfig(pt, pn, m, s);
}

void GpioPin::copy(GpioPin &o)
{
    this->pin = o.pin;
    this->pinNum = o.pinNum;
    this->port = o.port;
}

bool GpioPin::available() { return this->port != nullptr; }

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

bool GpioPin::getOutputState(void) { return port->ODR & (this->pin); }
void GpioPin::set(void) { this->port->BSRR = this->pin; }
void GpioPin::reset(void) { this->port->BRR = this->pin; }
void GpioPin::flip(void) { this->operator=(!((this->port->ODR) & (this->pin))); }
bool GpioPin::read(void) { return this->port->IDR & this->pin; }
void GpioPin::wait(bool s)
{
    while (this->read() != s)
        ;
}
// void GpioPin::operator=(bool s) { s ? (this->port->ODR |= this->pin) : (this->port->ODR &= (~this->pin)); }
void GpioPin::operator=(bool s) { s ? this->set() : this->reset(); }
GpioPin &GpioPin::operator=(GPIO::GpioPin &o)
{
    this->operator=(o.read());
    return *this;
}
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

static uint16_t _GPIO_ExtiFlag;

#if 0
#define __EXTIx_IRQHandler(__EXTIx)                        \
    if (EXTI->PR & __EXTI_LINE##__EXTIx)                   \
        if (EXTI->IMR & __EXTI_LINE##__EXTIx)              \
        {                                                  \
            EXTI->PR = __EXTI_LINE##__EXTIx;               \
            if (__GPIO_EXTI_Callbacks[__EXTIx] != nullptr) \
                __GPIO_EXTI_Callbacks[__EXTIx]();          \
            else                                           \
                _GPIO_ExtiFlag |= (__EXTI_LINE##__EXTIx);  \
        }
#else
void __EXTIx_IRQHandler(uint8_t x)
{
    uint32_t line = 0x01 << x;
    if (EXTI->PR & line)
        if (EXTI->IMR & line)
        {
            EXTI->PR = line;
            if (__GPIO_EXTI_Callbacks[x] != nullptr)
                __GPIO_EXTI_Callbacks[x]();
            else
                _GPIO_ExtiFlag |= (line);
        }
}
#endif

// template <uint8_t _PIN>
// void __GPIO_SetFlag(void)
// {
//     uint16_t mask = 0x01;
//     mask <<= _PIN;
//     _GPIO_ExtiFlag |= mask;
// }

// void (*const __GPIO_EXTI_FLG_FUNs[16])(void) = {
//     __GPIO_SetFlag<0>, __GPIO_SetFlag<1>, __GPIO_SetFlag<2>, __GPIO_SetFlag<3>,
//     __GPIO_SetFlag<4>, __GPIO_SetFlag<5>, __GPIO_SetFlag<6>, __GPIO_SetFlag<7>,
//     __GPIO_SetFlag<8>, __GPIO_SetFlag<9>, __GPIO_SetFlag<10>, __GPIO_SetFlag<11>,
//     __GPIO_SetFlag<12>, __GPIO_SetFlag<13>, __GPIO_SetFlag<14>, __GPIO_SetFlag<15>};

void GpioPin::setExti(void (*callback)(void), uint8_t priority)
{
    // __GPIO_EXTI_Callbacks[this->pinNum] = __GPIO_EXTI_FLG_FUNs[this->pinNum];
    __GPIO_EXTI_Callbacks[this->pinNum] = callback;

    uint8_t pn = this->pinNum;
    uint8_t tmp, portIdx;
    uint32_t mask1, mask2;

    __EXTI_RCC_EN();
    portIdx = this->portNum;
    tmp = pn;
    tmp >>= 2;
    mask1 = 0x0f;
    mask2 = portIdx;

    portIdx = (pn % 4) << 2;
    mask1 <<= portIdx;
    mask2 <<= portIdx;

    EXTI->CR[tmp] &= ~mask1;
    EXTI->CR[tmp] |= mask2;

    EXTI->IMR |= this->pin;

    // If there is no PULL-DOWN, then the FALLING edge needs to trigger
    if (this->currentMode != Mode_IPD)
        EXTI->FTSR |= this->pin;
    // If there is no PULL-UP, then the RAISING edge needs to trigger
    if (this->currentMode != Mode_IPU)
        EXTI->RTSR |= this->pin;

    // nvic
    IRQn_Type extiN;
    if (pn <= 1)
        extiN = EXTI0_1_IRQn;
    else if (pn <= 3)
        extiN = EXTI2_3_IRQn;
    else
        extiN = EXTI4_15_IRQn;

    NVIC_SetPriority(extiN, priority);
    NVIC_EnableIRQ(extiN);
}

bool GpioPin::isTriggered() { return this->pin & _GPIO_ExtiFlag; }
void GpioPin::triggerFlagReset() { _GPIO_ExtiFlag &= (~(this->pin)); }

#include "string.h"

extern "C"
{
    enum
    {
        KeyFsm_Idle = 0,
        KeyFsm_AntiPressedShake,
        KeyFsm_OnShort,
        KeyFsm_OnLong,
        KeyFsm_ShortAnti,
        KeyFsm_LongAnti,
        KeyFsm_mulWait,
        KeyFsm_mulAnti,
        KeyFsm_evtShort,
        KeyFsm_evtLong,
        // KeyFsm_evtMul
    };
    static inline void __KeyBaseInit(__KeyBaseHandle_t *handle)
    {
        memset(handle, 0, sizeof(__KeyBaseHandle_t));
        handle->fsm = KeyFsm_Idle;
    }

#define _KeyAntiShakeTick 2

    // __attribute__((noinline))
    static void __KeyBaseloopByMsTick(__KeyBaseHandle_t *handle, uint8_t triggered)
    {
        switch (handle->fsm)
        {
        case KeyFsm_Idle:
            if (triggered)
            {
                handle->fsm = KeyFsm_AntiPressedShake;
                handle->antiTickCnt = 0;
            }
            break;
        case KeyFsm_AntiPressedShake:
            if (triggered)
            {
                ++(handle->antiTickCnt);
                if (handle->antiTickCnt >= _KeyAntiShakeTick)
                {
                    handle->fsm = KeyFsm_OnShort;
                    handle->longTickCnt = 0;
                }
            }
            else
                handle->fsm = KeyFsm_Idle;
            break;
        case KeyFsm_OnShort:
            if (triggered)
            {
                ++(handle->longTickCnt);
                if ((handle->longTickCnt >= 1000)) //&& (handle->mulCnt == 0))
                {
                    handle->fsm = KeyFsm_OnLong;
                }
            }
            else
            {
                handle->fsm = KeyFsm_ShortAnti;
                handle->antiTickCnt = 0;
            }
            break;
        case KeyFsm_OnLong:
            if (!triggered)
            {
                handle->fsm = KeyFsm_LongAnti;
                handle->antiTickCnt = 0;
                handle->longTickCnt = 0;
            }
            break;
        case KeyFsm_ShortAnti:
            if (triggered)
                handle->fsm = KeyFsm_OnShort;
            else
            {
                ++(handle->antiTickCnt);
                if (handle->antiTickCnt >= _KeyAntiShakeTick)
                {
                    if (handle->mulCb)
                    {
                        handle->fsm = KeyFsm_mulWait;
                        handle->antiTickCnt = 0;
                        handle->longTickCnt = 0;
                    }
                    else
                        handle->fsm = KeyFsm_evtShort;
                }
            }
            break;
        case KeyFsm_LongAnti:
            if (triggered)
                handle->fsm = KeyFsm_OnLong;
            else
            {
                ++(handle->antiTickCnt);
                if (handle->antiTickCnt >= _KeyAntiShakeTick)
                    handle->fsm = KeyFsm_evtLong;
            }
            break;

        case KeyFsm_mulWait:
            if (triggered)
            {
                ++(handle->mulCnt);
                handle->fsm = KeyFsm_mulAnti;
                handle->antiTickCnt = 0;
            }
            else
            {
                ++(handle->antiTickCnt);
                if (handle->antiTickCnt >= 50)
                    handle->fsm = KeyFsm_evtShort;
            }

            break;
        case KeyFsm_mulAnti:
            if (triggered)
            {
                ++(handle->antiTickCnt);
                if (handle->antiTickCnt >= _KeyAntiShakeTick)
                    handle->fsm = KeyFsm_OnShort;
            }
            else
            {
                --(handle->mulCnt);
                handle->antiTickCnt = 0;
                handle->fsm = KeyFsm_mulWait;
            }
            break;
        case KeyFsm_evtShort:
            if ((handle->mulCnt) && (handle->mulCb))
                handle->mulCb(handle->mulCnt, handle->arg);
            else if (handle->cb)
                handle->cb(KeyPressKind_Short, handle->arg);
            handle->mulCnt = 0;
            handle->fsm = KeyFsm_Idle;
            break;

        case KeyFsm_evtLong:
            if (handle->cb)
                handle->cb(KeyPressKind_Long, handle->arg);
            handle->mulCnt = 0;
            handle->fsm = KeyFsm_Idle;
            break;

        default:
            break;
        }
    }
}

KeyDualEdge::KeyDualEdge(const char *pinName)
{
    __GPIO_PinName2PinData(pinName, this->gpio, this->pinNum);
    modeConfig(this->gpio, this->pinNum, Mode_IPD);
    // timestamp = sys::getTimeStamp();
    this->crtEdge = 0;
    this->gpio->BRR = (0x01 << (this->pinNum));
    __KeyBaseInit(&(this->highTrig));
    __KeyBaseInit(&(this->lowTrig));
}

void KeyDualEdge::setHighCbk(void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *))
{
    this->highTrig.cb = cbk;
    this->highTrig.mulCb = mul;
    this->highTrig.arg = arg;
}

void KeyDualEdge::setLowCbk(void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *))
{
    this->lowTrig.cb = cbk;
    this->lowTrig.mulCb = mul;
    this->lowTrig.arg = arg;
}

void KeyDualEdge::loopTick()
{
    // uint32_t currentTimeGap;
    // currentTimeGap = sys::getTimeStamp() - timestamp;
    // if (currentTimeGap >= 2)
    // {
    uint16_t pin = 0x01 << (this->pinNum);
    uint8_t level;
    // timestamp += currentTimeGap;
    level = (this->gpio->IDR & pin) ? 1 : 0;
    if (this->crtEdge)
    {
        __KeyBaseloopByMsTick(&(this->lowTrig), !level);
        this->gpio->BRR = pin;
        this->crtEdge = 0;
    }
    else
    {
        __KeyBaseloopByMsTick(&(this->highTrig), level);
        this->gpio->BSRR = pin;
        this->crtEdge = 1;
    }
    // }
}

Key::Key(const char *pinName, uint8_t activeLevel, void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *))
{
    __GPIO_PinName2PinData(pinName, this->gpio, this->pinNum);
    this->activeLevel = (activeLevel ? 1 : 0);
    if (this->activeLevel)
        modeConfig(this->gpio, this->pinNum, Mode_IPD);
    else
        modeConfig(this->gpio, this->pinNum, Mode_IPU);

    __KeyBaseInit(&(this->base));
    this->base.cb = cbk;
    this->base.arg = arg;
    this->base.mulCb = mul;
}

bool Key::isActive()
{
    uint16_t pin = 0x01 << (this->pinNum);
    uint8_t level;
    level = ((this->gpio->IDR & pin) ? 1 : 0);
    return (level == (this->activeLevel));
}

void Key::loopTick()
{
    uint16_t pin = 0x01 << (this->pinNum);
    uint8_t level;
    level = (this->gpio->IDR & pin) ? 1 : 0;
    __KeyBaseloopByMsTick(&(this->base), (level == (this->activeLevel)));
}

extern "C"
{
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
        uint32_t line = 0x01 << 3;
        uint8_t x = 3;
        while (x < 15)
        {
            line <<= 1;
            ++x;
            if (EXTI->PR & line)
                if (EXTI->IMR & line)
                {
                    EXTI->PR = line;
                    if (__GPIO_EXTI_Callbacks[x] != nullptr)
                        __GPIO_EXTI_Callbacks[x]();
                    else
                        _GPIO_ExtiFlag |= (line);
                }
        }

        // __EXTIx_IRQHandler(4);
        // __EXTIx_IRQHandler(5);
        // __EXTIx_IRQHandler(6);
        // __EXTIx_IRQHandler(7);
        // __EXTIx_IRQHandler(8);
        // __EXTIx_IRQHandler(9);
        // __EXTIx_IRQHandler(10);
        // __EXTIx_IRQHandler(11);
        // __EXTIx_IRQHandler(12);
        // __EXTIx_IRQHandler(13);
        // __EXTIx_IRQHandler(14);
        // __EXTIx_IRQHandler(15);
    }
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
        __SYSCFG_RCC_EN();
        line = 0x01 << pn;

        tmp = pn;
        tmp >>= 2;
        mask1 = 0x0f;
        mask2 = portIdx;

        portIdx = (pn % 4) << 2;
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
            // mask1 = (uint32_t)EXTI0_1_IRQHandler;
        }
        else if (pn <= 3)
        {
            extiN = EXTI2_3_IRQn;
            // mask1 = (uint32_t)EXTI2_3_IRQHandler;
        }
        else
        {
            extiN = EXTI4_15_IRQn;
            // mask1 = (uint32_t)EXTI4_15_IRQHandler;
        }

        // NVIC_SetVector(extiN, mask1);
        NVIC_SetPriority(extiN, 1);
        NVIC_EnableIRQ(extiN);
        // exti cfg
    }
}
