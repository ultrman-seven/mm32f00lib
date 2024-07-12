#include "stdint.h"
#include "list"
#include "reg_common.h"
#include "core_cm0.h"
#include "reg_rcc.h"
#include "wySys.hpp"

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

namespace sys
{
    uint32_t GetSysClockFreq(void)
    {
        uint32_t result;
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
    }
    uint32_t GetHCLKFreq(void)
    {
        return (GetSysClockFreq() >> tbPresc[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]);
    }
    uint32_t GetPCLK1Freq(void)
    {
        return (GetHCLKFreq() >> tbPresc[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]);
    }
    uint32_t getTimeStamp(void)
    {
        return __sysMsTimeStamp;
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
