#ifndef __MM32_CPP_HAL_WY_LIB_SYS_HPP__
#define __MM32_CPP_HAL_WY_LIB_SYS_HPP__
#include "stdint.h"
namespace sys
{

    enum class hClkDivEnum
    {
        divNone = 0,
        div2 = 0x08,
        div4,
        div8,
        div16,
        div64,
        div128,
        div256,
        div512
    };

    void deleteLoop(void (*f)(void));
    void runLoopFunctions(void);
    void throwFunc2Loop(void (*)(void));
    void delayMs(uint32_t ms);
    uint32_t getTimeStamp(unsigned long *t);
    uint32_t getTimeStamp(void);
    void delayBreak(void);
    uint32_t GetSysClockFreq(void);
    uint32_t GetPCLK1Freq(void);
    uint32_t GetHCLKFreq(void);
    void setHclkDiv(hClkDiv::hClkDivEnum d);
}
#endif /* __MM32_CPP_HAL_WY_LIB_SYS_HPP__ */
