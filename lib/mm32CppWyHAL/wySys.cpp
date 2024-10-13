#include "stdint.h"
#include "list"
#include "reg_common.h"
#include "core_cm0.h"
#include "reg_rcc.h"
#include "wySys.hpp"
#include "cppHalReg.hpp"

std::list<void (*)(void)> __mainLoopFuncs;
uint32_t __sysMsTimeStamp;
volatile uint32_t __sysDelayCnt;

void SysTick_Handler(void)
{
    ++__sysMsTimeStamp;
    if (__sysDelayCnt)
        --__sysDelayCnt;
}

void (*__sysFoo)(void);
bool __funcListRmvIf(void (*a)(void))
{
    return a == __sysFoo;
}

#include "mm32_device.h"
const uint8_t tbPresc[] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
uint32_t __sysIrqMask;

namespace sys
{
    inline void irqDisable()
    {
    }
    uint32_t GetSysClockFreq(void)
    {
#if defined __MM32F00_
        switch (RCC->CFGR & RCC_CFGR_SWS)
        {
        case RCC_CFGR_SWS_LSI:
            return LSI_VALUE;

        case RCC_CFGR_SWS_HSE:
            return HSE_VALUE;

        case RCC_CFGR_SWS_HSI:
            return HSI_48MHz;

        default:
            return HSI_48MHz_DIV6;
        }
        return HSI_48MHz_DIV6;
#elif defined __MM32F02_
        uint32_t result;
        uint32_t clock, mul, div;
        switch (RCC->CFGR & RCC_CFGR_SWS)
        {

        case RCC_CFGR_SWS_HSE:
            result = HSE_VALUE;
            break;

        case RCC_CFGR_SWS_PLL:
            clock = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC) ? (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLXTPRE) ? (HSE_VALUE >> 1) : HSE_VALUE)
                                                               : HSI_VALUE_PLL_ON;
            mul = ((RCC->PLLCFGR & (u32)RCC_PLLCFGR_PLL_DN) >> RCC_PLLCFGR_PLL_DN_Pos) + 1;
            div = ((RCC->PLLCFGR & RCC_PLLCFGR_PLL_DP) >> RCC_PLLCFGR_PLL_DP_Pos) + 1;

            result = clock * mul / div;
            break;
        default:
            result = HSI_DIV6;
            break;
        }
        return result;
#else
        return 0;
#endif
    }
    uint32_t GetHCLKFreq(void)
    {
        return (GetSysClockFreq() >> tbPresc[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]);
    }
    uint32_t GetPCLK1Freq(void)
    {
        return (GetHCLKFreq() >> tbPresc[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]);
    }
    uint32_t getTimeStamp(void) { return __sysMsTimeStamp; }
    void resetTimeStamp(void)
    {
        NVIC_DisableIRQ(SysTick_IRQn);
        __sysMsTimeStamp = 0;
        NVIC_SetPriority(SysTick_IRQn, 0);
        NVIC_EnableIRQ(SysTick_IRQn);
    }

    uint32_t getTimeStamp(unsigned long *t)
    {
        *t = __sysMsTimeStamp;
        return __sysMsTimeStamp;
    }
    void throwFunc2Loop(void (*f)(void))
    {
        __mainLoopFuncs.push_back(f);
    }
    void runLoopFunctions(void)
    {
        for (auto i : __mainLoopFuncs)
            if (i != nullptr)
                i();
    }
    void deleteLoop(void (*f)(void))
    {
        __sysFoo = f;
        __mainLoopFuncs.remove_if(__funcListRmvIf);
    }
    void delayMs(uint32_t ms)
    {
        __sysDelayCnt = ms;
        while (__sysDelayCnt)
            ;
    }
    void delayBreak(void)
    {
        __sysDelayCnt = 0;
    }
    void setHclkDiv(hClkDiv d)
    {
        uint8_t div = (uint8_t)d;
        RCC->CFGR &= 0xffffff0f;
        if (div)
            RCC->CFGR |= (div << 4);
        SysTick_Config(GetHCLKFreq() / 1000);
        NVIC_SetPriority(SysTick_IRQn, 0);
    }
} // namespace sys

using sys::GetSysClockFreq;

extern "C"
{
    void SystemInit();
    void SystemInitWy(void)
    {
        SystemInit();
        __sysMsTimeStamp = 0;
        __sysDelayCnt = 0;
        // SysTick_Config(SystemCoreClock / 1000);
        SysTick_Config(GetSysClockFreq() / 1000);
        NVIC_SetPriority(SysTick_IRQn, 0);
    }
}
