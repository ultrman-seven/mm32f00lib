#ifndef D8F0D8F2_5157_4074_83FF_4622987C0803
#define D8F0D8F2_5157_4074_83FF_4622987C0803

#define __MM32F02_

// #include "stdint.h"
#include "reg_rcc.h"
#include "reg_common.h"
#include "core_cm0.h"
/// GPIO
////
extern const uint32_t __GPIO_PORT_BASEs[];
extern const uint32_t __GPIO_PORT_RCC_EN[];
#define __RCC_GPIO_ENR (RCC->AHBENR)
#define __EXTI_RCC_EN() (RCC->APB2ENR |= RCC_APB2ENR_EXTI)
#define __SYSCFG_RCC_EN() (RCC->APB2ENR |= RCC_APB2ENR_SYSCFG)

#define __ADC_RCC_EN() (RCC->APB2ENR |= RCC_APB2ENR_ADC1)

/// uart
extern const uint32_t __UART_BASEs[];
// extern const uint32_t __UART_RCC_EN[];
// #define __RCC_UART_ENR (*((uint32_t *)(RCC_BASE + 0x1c)))
extern const uint16_t __UART_RCC_ENR_CFG_OFFSETs[];
extern IRQn_Type const __UART_IRQ[];
extern uint16_t const *const __UART_Rx_GPIO_AFs[];
extern uint16_t const *const __UART_Tx_GPIO_AFs[];
extern uint32_t const __UART_BUS_CFG;

/// tim
extern const uint32_t __TIM_IDXs[];
extern const uint32_t __TIM_BASEs[];
extern const uint32_t __TIM_RCC_EN[];
extern IRQn_Type const __TIM_IRQ[];
#define __RCC_TIM_ENR (*((uint32_t *)(RCC_BASE + 0x1C)))

#endif
