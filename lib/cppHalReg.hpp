#ifndef C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531
#define C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531

#define __MM32F00_

// #include "stdint.h"
#include "reg_rcc.h"
#include "reg_common.h"
#include "core_cm0.h"
/// GPIO
////
extern const uint32_t __GPIO_PORT_BASEs[];
extern const uint32_t __GPIO_PORT_RCC_EN[];
#define __RCC_GPIO_ENR (*((uint32_t *)(RCC_BASE + 0x14)))
#define __EXTI_RCC_EN() (*(uint32_t *)(RCC_BASE + 0x1c) |= RCC_APB1ENR_EXTI)

#define __ADC_RCC_EN() (RCC->APB1ENR |= RCC_APB1ENR_ADC1)

/// uart
extern const uint32_t __UART_BASEs[];
extern const uint32_t __UART_RCC_EN[];
#define __RCC_UART_ENR (*((uint32_t *)(RCC_BASE + 0x1c)))
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

#endif /* C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531 */
