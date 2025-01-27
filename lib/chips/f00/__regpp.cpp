#include "mm32_reg.h"
#include "wyGpio.hpp"
#include "cppHalReg.hpp"
#include "wySys.hpp"

const uint32_t __GPIO_PORT_BASEs[] = {GPIOA_BASE, GPIOB_BASE};
const uint32_t __GPIO_PORT_RCC_EN[] = {RCC_AHBENR_GPIOA, RCC_AHBENR_GPIOB};

const uint32_t __TIM_BASEs[] = {TIM1_BASE, TIM3_BASE, TIM14_BASE};
const uint32_t __TIM_RCC_EN[] = {RCC_APB1ENR_TIM1, RCC_APB1ENR_TIM3, RCC_APB1ENR_TIM14};

const uint32_t __UART_BASEs[] = {UART1_BASE, UART2_BASE};
const uint32_t __UART_RCC_EN[] = {RCC_APB1ENR_UART1, RCC_APB1ENR_UART2};

#define __RCC_APB1ENR_OFFSET 0x1c

const uint16_t __UART_RCC_ENR_CFG_OFFSETs[] = {
    ((__RCC_APB1ENR_OFFSET) << 8) + RCC_APB1ENR_UART1_Pos,
    ((__RCC_APB1ENR_OFFSET) << 8) + RCC_APB1ENR_UART2_Pos};

#define __UART1_BUS sys::_MCU_BUS_APB1
#define __UART2_BUS sys::_MCU_BUS_APB1

uint32_t const __UART_BUS_CFG = __UART1_BUS + (__UART2_BUS << 2);

const uint8_t __TIM_IDXs[] = {
    0, 6, 1, 6, 6,
    6, 6, 6, 6, 6,
    6, 6, 6, 2};

const uint16_t __UART1_Rx_GPIO_AFs[] = {__GPIO_AF_Val(0, 0, 1), __GPIO_AF_Val(0, 3, 1), __GPIO_AF_Val(0, 13, 1), 0xffff};
const uint16_t __UART1_Tx_GPIO_AFs[] = {__GPIO_AF_Val(0, 12, 1), __GPIO_AF_Val(0, 14, 1), 0xffff};
const uint16_t __UART2_Rx_GPIO_AFs[] = {__GPIO_AF_Val(0, 13, 2), 0xffff};
const uint16_t __UART2_Tx_GPIO_AFs[] = {__GPIO_AF_Val(0, 1, 2), 0xffff};

uint16_t const *const __UART_Rx_GPIO_AFs[] = {__UART1_Rx_GPIO_AFs, __UART2_Rx_GPIO_AFs};
uint16_t const *const __UART_Tx_GPIO_AFs[] = {__UART1_Tx_GPIO_AFs, __UART2_Tx_GPIO_AFs};

IRQn_Type const __UART_IRQ[] = {UART1_IRQn, UART2_IRQn};
IRQn_Type const __TIM_IRQ[] = {TIM1_BRK_UP_TRG_COM_IRQn, TIM3_IRQn, TIM14_IRQn};
