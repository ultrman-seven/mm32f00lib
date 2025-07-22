#include "wyAdc.hpp"
#include "wyGpio.hpp"
// #include "reg_rcc.h"
#include "cppHalReg.hpp"

using ADC::Channel;
using ADC::ChannelScan;

const uint16_t __ADC_PinChs[] = {
    __GPIO_AF_Val(0, 7, 7), __GPIO_AF_Val(0, 15, 6), __GPIO_AF_Val(0, 2, 5),
    __GPIO_AF_Val(0, 11, 4), __GPIO_AF_Val(0, 12, 3), __GPIO_AF_Val(0, 3, 2),
    __GPIO_AF_Val(1, 0, 1), __GPIO_AF_Val(1, 1, 0),
    0xffff};

uint16_t __AdcRes[9];

// 使能内部参考电压
#define __Adc_InnerRefEn() (ADC1->ADCFG |= 0x08)

#define AdcCheckConvertCompleteFlag() (ADC1->ADSTA & 0x01)
#define AdcResetConvertCompleteFlag() (ADC1->ADSTA |= 0x01)
#define AdcStartConvert() (ADC1->ADCR |= ADC_CR_ADST)

void (*__adc1IRQ_Callback)(void) = ADC::adcGetAllChsVal;

extern "C"
{
    void ADC1_IRQHandler(void)
    {
        if (ADC1->ADSTA & ADC_SR_ADIF)
        {
            ADC1->ADSTA |= ADC_SR_ADIF;

            // __adc1IRQ_Callback();
        }
    }
}

ChannelScan::ChannelScan() : chNum(0)
{
    __ADC_RCC_EN();
    // 12 bit 分辨率、16分频
    ADC1->ADCFG &= ~(ADC_CFGR_PRE | ADC_CFGR_RSLTCTL);
    ADC1->ADCFG |= (ADC_CFGR_PRE_16 | ADC_CFGR_RSLTCTL_12);

    // 右对齐、单次转换
    ADC1->ADCR &= ~(ADC_CR_ALIGN | ADC_CR_MODE | ADC_CR_TRGSEL);
    ADC1->ADCR |= (ADC_CR_RIGHT);

    this->regBackup_chAny0 = 0;
    this->regBackup_chAny1 = 0;
}

uint16_t Channel::getVal()
{
    return __AdcRes[idx];
}

Channel::Channel(const char *pinName)
{
    if (*pinName == 'v' || *pinName == 'V')
    {
        __Adc_InnerRefEn();
        idx = 8;
    }
    else
    {
        GPIO::modeConfig(pinName, GPIO::Mode_AIN);
        idx = GPIO::afTable2afVal(pinName, __ADC_PinChs);
    }
}

static inline uint8_t __ADC_add_channel(const char *pin, uint8_t chSelect)
{
    uint8_t ch;
    uint32_t mask;
    if (*pin == 'v' || *pin == 'V')
    {
        __Adc_InnerRefEn();
        ch = 8;
    }
    else
    {
        GPIO::modeConfig(pin, GPIO::Mode_AIN);
        ch = GPIO::afTable2afVal(pin, __ADC_PinChs);
    }

    if (0xff == ch)
        return chSelect;

    if (chSelect > 8)
        return chSelect;
    if (chSelect == 8)
    {
        ADC1->CHANY1 &= 0xfffffff0;
        ADC1->CHANY1 |= ch;
    }
    else
    {
        mask = ~(0x0f << (chSelect * 4));
        ADC1->CHANY0 &= mask;
        mask = (ch & 0x0f) << (chSelect * 4);
        ADC1->CHANY0 |= mask;
    }
    return chSelect + 1;
}

static inline uint8_t __ADC_add_channel(uint8_t ch, uint8_t chSelect)
{
    if (ch > 8)
        return chSelect;

    if (chSelect > 8)
        return chSelect;
    if (chSelect == 8)
    {
        ADC1->CHANY1 &= 0xfffffff0;
        ADC1->CHANY1 |= ch;
    }
    else
    {
        uint32_t mask = ~(0x0f << (chSelect * 4));
        ADC1->CHANY0 &= mask;
        mask = (ch & 0x0f) << (chSelect * 4);
        ADC1->CHANY0 |= mask;
    }
    return chSelect + 1;
}

ChannelScan::ChannelScan(char const *const *pinLists, uint8_t nums) : ChannelScan()
{
    uint8_t cnt;
    uint8_t ch;
    const char *pin;

    for (cnt = 0; cnt < nums; ++cnt)
        chNum = __ADC_add_channel(pinLists[cnt], chNum);
    this->regBackup_chAny0 = ADC1->CHANY0;
    this->regBackup_chAny1 = ADC1->CHANY1;
}

ChannelScan::ChannelScan(Channel const *chLists, uint8_t nums) : ChannelScan()
{
    uint8_t cnt;
    uint8_t ch;

    for (cnt = 0; cnt < nums; ++cnt)
        chNum = __ADC_add_channel(chLists[cnt].idx, chNum);
    this->regBackup_chAny0 = ADC1->CHANY0;
    this->regBackup_chAny1 = ADC1->CHANY1;
}

void ChannelScan::addChannel(Channel ch)
{
    chNum = __ADC_add_channel(ch.idx, chNum);
    this->regBackup_chAny0 = ADC1->CHANY0;
    this->regBackup_chAny1 = ADC1->CHANY1;
}

void ChannelScan::addChannel(const char *pinName)
{
    chNum = __ADC_add_channel(pinName, chNum);
    this->regBackup_chAny0 = ADC1->CHANY0;
    this->regBackup_chAny1 = ADC1->CHANY1;
}

void ChannelScan::configReg()
{
    if (chNum == 0)
        return;

    // 转换模式
    ADC1->ADCR &= ~(ADC_CR_MODE);
    ADC1->ADCR |= ADC_CR_SCAN;

    // 使能 ADC 转换
    ADC1->ADCFG |= 0x01;

    ADC1->ANYCR &= ~ADC1_CHANY_CR_MDEN; // 禁用多通道转换

    // 恢复 CHANY0 和 CHANY1 寄存器
    ADC1->CHANY0 = this->regBackup_chAny0;
    ADC1->CHANY1 = this->regBackup_chAny1;

    ADC1->ANYCFG &= ~(0x0f);     // 清除通道数量配置
    ADC1->ANYCFG |= (chNum - 1); // 设置通道数量

    // 使能多通道转换
    ADC1->ANYCR |= ADC1_CHANY_CR_MDEN;

    __adc1IRQ_Callback = adcGetAllChsVal; // 设置中断回调函数

    // 采样时间设置
    ADC1->CFGR &= ~ADC_CFGR_SAMCTL;
    // ADC1->CFGR |= ADC_CFGR_SAMCTL_13_5;
    ADC1->CFGR |= ADC_CFGR_SAMCTL_41_5;
    // // 配置中断
    // ADC1->ADCR |= ADC_CR_ADIE; // 使能 ADC 中断

    // // 配置NVIC
    // NVIC_SetPriority(ADC1_IRQn, 1);
    // NVIC_EnableIRQ(ADC1_IRQn);
}

void ChannelScan::start()
{
    AdcStartConvert();
}

uint8_t ChannelScan::isCompleted()
{
    // if (AdcCheckConvertCompleteFlag())
    // {
    //     AdcResetConvertCompleteFlag();
    //     return 1;
    // }
    if (ADC1->ADSTA & (ADC_SR_BUSY))
        return 0;
    return 1;
}

namespace ADC
{
    void adcGetAllChsVal(void)
    {
        uint8_t chNums;
        uint8_t ch;
        volatile uint32_t *adcDrPtr = &ADC1->ADDR0;
        uint32_t regChAny0;

        chNums = ADC1->ANYCFG & 0x0f;
        if (chNums == 8)
        {
            ch = (ADC1->CHANY1 & 0x0f);
            --chNums;
            __AdcRes[ch] = *(adcDrPtr + ch) & 0x0fff; // 8th channel is in CHANY1
        }
        ++chNums;
        regChAny0 = ADC1->CHANY0;
        while (chNums--)
        {
            ch = (regChAny0 >> (chNums * 4)) & 0x0f;
            __AdcRes[ch] = *(adcDrPtr + ch) & 0x0fff; // 0..7 channels are in CHANY0
        }
    }
    // ADC 时钟分频
    void setClkDiv(uint8_t div)
    {
    }

    // ADC 分辨率
    void setResolution(uint8_t r)
    {
    }

    // ADC 采样时间

    uint16_t getAdcVal(const char *pin)
    {
        uint16_t val = 0;
        uint8_t ch;
#ifdef __MM32F00_
        __ADC_RCC_EN();

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

        // 12 bit 分辨率、16分频
        ADC1->ADCFG &= ~(ADC_CFGR_PRE | ADC_CFGR_RSLTCTL);
        ADC1->ADCFG |= (ADC_CFGR_PRE_16 | ADC_CFGR_RSLTCTL_12);

        // 右对齐、单次转换
        ADC1->ADCR &= ~(ADC_CR_ALIGN | ADC_CR_MODE | ADC_CR_TRGSEL);
        ADC1->ADCR |= (ADC_CR_RIGHT | ADC_CR_IMM);

        // 采样时间设置
        ADC1->CFGR &= ~ADC_CFGR_SAMCTL;
        // ADC1->CFGR |= ADC_CFGR_SAMCTL_13_5;
        ADC1->CFGR |= ADC_CFGR_SAMCTL_41_5;
        // ADC1->CFGR |= ADC_CFGR_SAMCTL_239_5;

        // 转换使能
        ADC1->ADCFG |= 0x01;

        ADC1->ANYCR &= ~ADC1_CHANY_CR_MDEN;
        if (ch != 0xff)
        {
            ADC1->ANYCFG = 0;
            ADC1->CHANY0 &= 0xfffffff0;
            ADC1->CHANY0 |= ch;
            ADC1->ANYCR |= ADC1_CHANY_CR_MDEN;

            AdcStartConvert();

            while (!AdcCheckConvertCompleteFlag())
                ;
            AdcResetConvertCompleteFlag(); // this bit write 1 to reset. WTF!
            val = ADC1->ADDATA;
        }

        RCC->APB1ENR &= ~RCC_APB1ENR_ADC1;
#endif
        return val;
    }
} // namespace ADC
