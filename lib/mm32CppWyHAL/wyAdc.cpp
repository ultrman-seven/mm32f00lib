#include "wyAdc.hpp"
#include "wyGpio.hpp"
#include "reg_rcc.h"

const uint16_t __ADC_PinChs[] = {
    __GPIO_AF_Val(0, 7, 7), __GPIO_AF_Val(0, 15, 6), __GPIO_AF_Val(0, 2, 5),
    __GPIO_AF_Val(0, 11, 4), __GPIO_AF_Val(0, 12, 3), __GPIO_AF_Val(0, 3, 2),
    __GPIO_AF_Val(1, 0, 1), __GPIO_AF_Val(1, 1, 0), 0xffff};

namespace ADC
{
    uint16_t getAdcVal(const char *pin)
    {
        uint16_t val = 0;
        uint8_t ch;

        RCC->APB1ENR |= RCC_APB1ENR_ADC1;

        if (*pin == 'v' || *pin == 'V')
        {
            ADC1->ADCFG |= 0x08;
            ch = 8;
        }
        else
        {
            GPIO::modeConfig(pin, GPIO::Mode_AIN);
            ch = GPIO::afTable2afVal(pin, __ADC_PinChs);
        }

        ADC1->ADCFG &= ~(ADC_CFGR_PRE | ADC_CFGR_RSLTCTL);
        ADC1->ADCFG |= (ADC_CFGR_PRE_16 | ADC_CFGR_RSLTCTL_12);

        ADC1->ADCR &= ~(ADC_CR_ALIGN | ADC_CR_MODE | ADC_CR_TRGSEL);
        ADC1->ADCR |= (ADC_CR_RIGHT | ADC_CR_IMM);

        ADC1->CFGR &= ~ADC_CFGR_SAMCTL;
        // ADC1->CFGR |= ADC_CFGR_SAMCTL_13_5;
        ADC1->CFGR |= ADC_CFGR_SAMCTL_41_5;

        ADC1->ADCFG |= 0x01;

        ADC1->ANYCR &= ~ADC1_CHANY_CR_MDEN;
        if (ch != 0xff)
        {
            ADC1->ANYCFG = 0;
            ADC1->CHANY0 &= 0xfffffff0;
            ADC1->CHANY0 |= ch;
            ADC1->ANYCR |= ADC1_CHANY_CR_MDEN;

            ADC1->ADCR |= ADC_CR_ADST;

            while (!(ADC1->ADSTA & 0x01))
                ;
            ADC1->ADSTA |= 0x01; // this bit write 1 to reset. WTF!
            val = ADC1->ADDATA;
        }

        RCC->APB1ENR &= ~RCC_APB1ENR_ADC1;
        return val;
    }
} // namespace ADC
