#ifdef __cplusplus
extern "C"
{
#endif

#include "reg_common.h"
#include "core_cm0.h"

    /* External References */
    extern uint32_t __INITIAL_SP;
    extern uint32_t __STACK_LIMIT;
// extern uint32_t __heap_base;
// extern uint32_t __heap_limit;
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    extern uint32_t __STACK_SEAL;
#endif

#define Reset_Handler Reset_Handl

    extern __NO_RETURN void __PROGRAM_START(void);

    /* Internal References */
    __NO_RETURN void Reset_Handler(void);
    __NO_RETURN void Default_Handler(void);

    /* Cortex-M0 Exception Handlers */
    void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void HardFault_Handler(void) __attribute__((weak));
    void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
    void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

    /* Device-Specific Interrupt Handlers */
    /* To be added for each interrupt source as needed */
    void WWDG_IWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void EXTI0_1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void EXTI2_3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void EXTI4_15_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void ADC1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void TIM1_BRK_UP_TRG_COM_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void TIM14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void I2C1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void UART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
    void UART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/* Vector Table */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

    typedef void (*VECTOR_TABLE_Type)(void);

    /* Cortex-M0 Vector Table */
    const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
        (VECTOR_TABLE_Type)(&__INITIAL_SP), /* Initial Stack Pointer */
        Reset_Handler,           /* Reset Handler */
        NMI_Handler,             /* NMI Handler */
        HardFault_Handler,       /* Hard Fault Handler */
        MemManage_Handler,       /* MPU Fault Handler */
        BusFault_Handler,        /* Bus Fault Handler */
        UsageFault_Handler,      /* Usage Fault Handler */
        0,                       /* Reserved */
        0,                       /* Reserved */
        0,                       /* Reserved */
        0,                       /* Reserved */
        SVC_Handler,             /* SVCall Handler */
        DebugMon_Handler,        /* Debug Monitor Handler */
        0,                       /* Reserved */
        PendSV_Handler,          /* PendSV Handler */
        SysTick_Handler,         /* SysTick Handler */

        // 外设
        WWDG_IWDG_IRQHandler,
        PVD_IRQHandler,
        0,
        FLASH_IRQHandler,
        RCC_IRQHandler,
        EXTI0_1_IRQHandler,
        EXTI2_3_IRQHandler,
        EXTI4_15_IRQHandler,
        0,
        0,
        0,
        0,
        ADC1_IRQHandler,
        TIM1_BRK_UP_TRG_COM_IRQHandler,
        TIM1_CC_IRQHandler,
        0,
        TIM3_IRQHandler,
        0,
        0,
        TIM14_IRQHandler,
        0,
        0,
        0,
        I2C1_IRQHandler,
        0,
        SPI1_IRQHandler,
        0,
        UART1_IRQHandler,
        UART2_IRQHandler,
    };

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    /*---------------------------------------------------------------------------*/
    /* Reset Handler called on controller reset */

    extern void SystemInitWy(void);
    __NO_RETURN void Reset_Handler(void)
    {
        /* Initialize stack pointer and heap */
        __set_PSP((uint32_t)(&__INITIAL_SP));

        /* Initialize stack limit registers (for Armv8-M) */
        // __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
        // __set_PSPLIM((uint32_t)(&__STACK_LIMIT));

        /* Optional: Stack sealing for Armv8-M */
// #if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
//         __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
// #endif

        /* System Initialization */
        SystemInitWy(); /* CMSIS System Initialization */

        /* Start the C runtime (PreMain) */
        __PROGRAM_START(); /* Enter the PreMain (C library entry point) */
    }

    /* Default Handler for Exceptions / Interrupts */
    void Default_Handler(void)
    {
        while (1)
            ;
    }

    /*---------------------------------------------------------------------------*/
    /* Hard Fault Handler */
    void HardFault_Handler(void)
    {
        while (1)
            ; /* Infinite loop */
    }

#ifdef __cplusplus
}
#endif
