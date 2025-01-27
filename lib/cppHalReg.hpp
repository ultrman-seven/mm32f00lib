#ifndef C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531
#define C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531

#ifdef __Chip_MM32F02

#define __MM_Chip_Has_APB2
#define __UART_TotalNum 4
#include "chips/f02/__regpp.hpp"

#elif defined __Chip_MM32F04

#include "chips/f02/__regpp.hpp"

#else

#define __UART_TotalNum 2
#include "chips/f00/__regpp.hpp"

#endif

#endif /* C2C88BE9_2A1F_4F6F_A47E_E55CBA48A531 */
