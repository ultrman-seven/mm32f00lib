#include <stdint.h>

/* 定义堆栈和堆的大小 */
#define STACK_SIZE 0x300 // 堆栈大小
#define HEAP_SIZE 0x100  // 堆大小

#ifdef __cplusplus
extern "C"
{
#endif

/* 堆栈和堆内存区域 */
static uint8_t Stack_Mem[STACK_SIZE] __attribute__((aligned(8), used, section(".stack")));
static uint8_t Heap_Mem[HEAP_SIZE] __attribute__((aligned(8), used, section(".heap")));

/* 堆栈和堆边界 */
extern uint8_t __initial_sp;
extern uint8_t __heap_base;
extern uint8_t __heap_limit;

/* 函数声明 */
void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* 外部中断声明 */
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

extern uint8_t __Vectors_End;
extern uint8_t __Vectors_Size;

/* 中断向量表 */
// __attribute__((section(".isr_vector")))
__attribute__((section(".reset")))
const void *InterruptVectorTable[] = {
    &Stack_Mem[STACK_SIZE], // 堆栈顶地址
    Reset_Handler,          // 复位处理程序
    NMI_Handler,            // NMI
    HardFault_Handler,      // 硬故障
    MemManage_Handler,      // 存储器管理故障
    BusFault_Handler,       // 总线故障
    UsageFault_Handler,     // 用法故障
    0,
    0,
    0,
    0,                // 保留
    SVC_Handler,      // SVCall
    DebugMon_Handler, // 调试监视器
    0,                // 保留
    PendSV_Handler,   // PendSV
    SysTick_Handler,  // SysTick

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

/* 复位处理程序 */
void Reset_Handler(void)
{
    extern void SystemInitWy(void);
    extern int main(void);

    /* 初始化堆栈指针 */
    __asm volatile("MSR MSP, %0" : : "r"(&Stack_Mem[STACK_SIZE]));

    /* 调用系统初始化函数 */
    SystemInitWy();

    /* 跳转到主程序入口 */
    main();
}

/* 默认中断处理程序 */
void Default_Handler(void)
{
    while (1)
    {
        // 死循环等待调试
    }
}

#ifdef __cplusplus
}
#endif
