#ifndef FE917FA8_796D_4FA1_B4FC_2EA31B714E24
#define FE917FA8_796D_4FA1_B4FC_2EA31B714E24
#include "stdint.h"
namespace DMA
{
    class configUnit
    {
    public:
        uint32_t peripheralAddr; /*!< Specifies the peripheral base address for DMAy Channelx. */
        uint32_t memoryAddr;     /*!< Specifies the memory base address for DMAy Channelx. */
        uint32_t bufferSize;
        union
        {
            struct
            {
                uint32_t en : 1;
                uint32_t transferCompleteIE : 1;
                uint32_t halfTransferIE : 1;
                uint32_t transferErrorIE : 1;

                uint32_t dirMem2Per : 1;
                uint32_t circularMode : 1;
                uint32_t pInc : 1;
                uint32_t mInc : 1;

                uint32_t pByteSize : 2;
                uint32_t mByteSize : 2;

                uint32_t priorityLevel : 2;
                uint32_t m2m : 1;
            } channelConfig;
            uint32_t __valOfChannelConfig;
        };

        configUnit();
        ~configUnit() = default;
        void config(uint8_t n = 1, uint8_t channel = 1);
    };

    void stop(uint8_t n, uint8_t channel);
    void start(uint8_t n, uint8_t channel);
} // namespace DMA

#endif /* FE917FA8_796D_4FA1_B4FC_2EA31B714E24 */
