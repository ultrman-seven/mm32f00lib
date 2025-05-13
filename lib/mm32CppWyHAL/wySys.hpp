#ifndef __MM32_CPP_HAL_WY_LIB_SYS_HPP__
#define __MM32_CPP_HAL_WY_LIB_SYS_HPP__
#include "stdint.h"
namespace sys
{

    inline void irqDisable();
    inline void irqEnable();
    inline void irqRecover();

    enum class hClkDiv
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

    enum __mcuBus
    {
        _MCU_BUS_AHB = 1,
        _MCU_BUS_APB1,
        _MCU_BUS_APB2
    };

    void deleteLoop(void (*f)(void));
    void runLoopFunctions(void);
    void throwFunc2Loop(void (*)(void));
    void delayMs(uint32_t ms);
    void delayUs(uint32_t us);
    uint32_t getTimeStamp(unsigned long *t);
    uint32_t getTimeStamp(void);
    void resetTimeStamp(void);
    void delayBreak(void);
    uint32_t GetSysClockFreq(void);
    uint32_t GetPCLK1Freq(void);
    uint32_t GetPCLK2Freq(void);
    uint32_t GetHCLKFreq(void);
    void setHclkDiv(hClkDiv d);

    class taskMsPeriod
    {
    private:
        uint32_t stamp;
        uint16_t timeGap;
        void (*callback)(void);

    public:
        bool triggered;
        taskMsPeriod(uint16_t msTime, void (*callback)(void) = nullptr);
        void loop();
        // ~timeTrigger();
    };

    class taskMsDelay
    {
    private:
        uint32_t stamp;
        uint16_t timeGap;
        uint8_t state;
        void (*callback)(void);

    public:
        taskMsDelay();
        void start(uint16_t msTime, void (*callback)(void) = nullptr);
        void loop();
    };
}

extern "C"
{
    void SystemInitWy(void);
}

#endif /* __MM32_CPP_HAL_WY_LIB_SYS_HPP__ */
