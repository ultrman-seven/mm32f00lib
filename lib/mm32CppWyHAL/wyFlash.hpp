#ifndef BDD3BB3E_4AC2_449A_B31A_278400B7E240
#define BDD3BB3E_4AC2_449A_B31A_278400B7E240

#include "stdint.h"
namespace flash
{
    class FlashBlock
    {
        private:
    // public:
        uint8_t dataLen;
        uint8_t mkStrLen;
        uint16_t addressOffset = 0;
        const uint16_t blockLen;
        uint32_t addressStart;
        char const *const mkStr;

        uint8_t deflowerVirgin();
        uint8_t scanTill0xff();
        void eraseSector();

        void lock(void);
        void unlock(void);
        void resetBlock(void);
        uint8_t program(uint8_t *data, uint8_t dataLen);

    public:
        FlashBlock(char const *const markStr, uint32_t dataLen, uint32_t startAdd = 0x08003c00, const uint32_t blockLen = 1024);
        bool isEmpty();
        uint8_t load(void *data);
        uint8_t save(void *data);
        // ~FlashBlock();
    };
} // namespace flash

#define FlashStartAddress 0x8000000
#define FlashPageSize 1024
#define FlashPageN(_n) (FlashStartAddress + (_n) * (FlashPageSize))

#endif /* BDD3BB3E_4AC2_449A_B31A_278400B7E240 */
