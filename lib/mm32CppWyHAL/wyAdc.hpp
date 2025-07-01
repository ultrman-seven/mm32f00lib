#ifndef E0EE71C5_1BE0_4B15_BE34_B7DDD8CDB4A6
#define E0EE71C5_1BE0_4B15_BE34_B7DDD8CDB4A6
#include "reg_adc.h"
#include "stdint.h"
namespace ADC
{
    class Channel
    {
    private:
    public:
        uint8_t idx;
        Channel(const char *pinName);
        uint16_t getVal();
    };

    class ChannelScan
    {
    private:
        uint32_t regBackup_chAny0;
        uint32_t regBackup_chAny1;

    public:
        uint8_t chNum;
        ChannelScan();
        ChannelScan(char const *const *pinLists, uint8_t nums);
        ChannelScan(Channel const *chLists, uint8_t nums);
        // ~ChannelScan();

        void addChannel(Channel ch);
        void addChannel(const char *pinName);
        void start();
        void configReg();
        uint8_t isCompleted();
    };

    uint16_t getAdcVal(const char *pin);
    void adcGetAllChsVal(void);
} // namespace ADC

#endif /* E0EE71C5_1BE0_4B15_BE34_B7DDD8CDB4A6 */
