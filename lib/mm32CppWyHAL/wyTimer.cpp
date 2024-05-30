#include "wyTimer.hpp"
#include "wySys.hpp"
#include "wyGpio.hpp"
// #include "reg_rcc.h"

using namespace TIM;

Timer::Timer(uint8_t timx, uint32_t period, uint32_t psc, uint8_t div, Mode m)
{
    this->numIdx = __TIM_IDXs[timx - 1];
    this->tim = (TIM_TypeDef *)__TIM_BASEs[numIdx];
    __RCC_TIM_ENR |= __TIM_RCC_EN[numIdx];

    tim->CR1 |= (0x80 | (div << 8));
    tim->ARR = period;
    tim->PSC = psc;
    tim->EGR = 1;
    tim->CR1 |= 0x01;
}
Timer::Timer(uint8_t timx, uint32_t usPeriod)
{
    uint32_t timSysClock = sys::GetPCLK1Freq();
    uint32_t arr, psc, div = 0;

    this->numIdx = __TIM_IDXs[timx - 1];
    this->tim = (TIM_TypeDef *)__TIM_BASEs[numIdx];
    __RCC_TIM_ENR |= __TIM_RCC_EN[numIdx];

    if (usPeriod >= 0xffff)
    {
        psc = timSysClock / 1000;
        if (psc >= 0xffff)
        {
            div = 3;
            psc >>= 2;
        }
        --psc;
        arr = usPeriod / 1000;
    }
    else
    {
        psc = timSysClock / 1000000 - 1;
        arr = usPeriod;
    }

    tim->CR1 |= (0x80 | (div << 8));
    tim->ARR = arr;
    tim->PSC = psc;
    tim->EGR = 1;
    tim->CR1 |= 0x01;
}

Timer::~Timer()
{
}

bool Timer::updated()
{
    if (tim->SR & 0x01)
    {
        tim->SR &= 0xfffffffe;
        return true;
    }
    return false;
}

const uint16_t __TIMx_ChxP_AFs[][4][4] = {
    {{__GPIO_AF_Val(0, 6, 1), __GPIO_AF_Val(0, 9, 2), 0xffff},
     {__GPIO_AF_Val(0, 8, 1), __GPIO_AF_Val(0, 10, 2), __GPIO_AF_Val(0, 11, 2), 0xffff},
     {__GPIO_AF_Val(0, 10, 1), __GPIO_AF_Val(0, 6, 4), 0xffff},
     {__GPIO_AF_Val(0, 7, 4), 0xffff}},
    {{},
     {},
     {},
     {}},
    {{},
     {},
     {},
     {}},
    {{},
     {},
     {},
     {}}};
const uint16_t __TIMx_ChxN_AFs[][4][4] = {
    {{},
     {},
     {},
     {}},
    {{},
     {},
     {},
     {}},
    {{},
     {},
     {},
     {}},
    {{},
     {},
     {},
     {}}};

pwmType Timer::setPwm(uint8_t ch, const char *p, const char *n)
{
    uint32_t *ccr;
    // uint8_t *ccmr;
    uint8_t af;
    uint32_t mask = 0x60;

    --ch;
    ccr = ((uint32_t *)&(tim->CCR1)) + ch;
    // ccmr = (uint8_t *)&(tim->CCMR1);
    // ccmr += (ch & 0x01);
    // if (ch >= 2)
    //     ccmr += 4;

    if (ch & 0x01)
        mask <<= 8;
    if (ch & 0x02)
        tim->CCMR2 |= mask;
    else
        tim->CCMR1 |= mask;

    *ccr = 0;
    // *ccr = tim->ARR >> 1;
    // *ccmr = 0x60;

    if (p != nullptr)
    {
        af = GPIO::afTable2afVal(p, __TIMx_ChxP_AFs[numIdx][ch]);
        if (af != 0xff)
            GPIO::afConfig(p, af, GPIO::Mode_AF_PP);
        tim->CCER |= (0x01 << (ch << 2));
    }
    if (n != nullptr)
    {
        // GPIO::afConfig(n, af, GPIO::Mode_AF_PP);
        tim->CCER |= (0x04 << (ch << 2));
    }

    tim->CR1 |= 0x80;
    tim->BDTR |= 0x8000;
    tim->CR1 |= 0x01;
    return ccr;
}
