#ifndef BDD3BB3E_4AC2_449A_B31A_278400B7E240
#define BDD3BB3E_4AC2_449A_B31A_278400B7E240

#include "stdint.h"
namespace flash
{

    class __FlashBase
    {
    private:
        char const *const mkStr;

    protected:
        uint8_t mkStrLen;
        const uint16_t blockLen;
        uint32_t addressStart;
        uint16_t addressOffset = 0;

        uint8_t program(uint8_t *data, uint8_t dataLen);
        uint8_t scanTill0xff(uint8_t emptyDataUnitSize);
        void resetBlock(void);
        uint8_t deflowerVirgin();

    public:
        __FlashBase(char const *const markStr, uint32_t startAdd, const uint32_t blockLen);
    };

    // For compatibility with older versions
    typedef class paraSaver FlashBlock;

    class paraSaver : public __FlashBase
    {
    private:
        uint8_t dataLen;

    public:
        paraSaver(char const *const markStr, uint32_t dataLen, uint32_t startAdd = 0x08003c00, const uint32_t blockLen = 1024);
        bool isEmpty();
        uint8_t load(void *data);
        uint8_t save(void *data);
        // ~FlashBlock();
    };

    class FlashArray : public __FlashBase
    {
    private:
        uint8_t unitSize;

    public:
        uint16_t arrayLen;
        FlashArray(const char *mark, uint8_t unitSize, uint32_t startAdd = 0x08003c00, const uint32_t blockLen = 1024);
        bool append(uint8_t *dat);
        void clearAll(void);
        bool searchData(uint8_t *data);
        // ~FlashArray();
    };

    class Counter : public __FlashBase
    {
    private:
        inline void add();
        uint16_t markVal;
        uint32_t cnt;

    public:
        Counter(const char *mark, uint32_t addressStart, uint32_t BlockLen);
        uint32_t operator++();
        uint32_t operator++(int);
        uint32_t getCount();
        void reset();
        // ~Counter();
    };
} // namespace flash

#define FlashStartAddress 0x8000000
#define FlashPageSize 1024
#define FlashPageN(_n) (FlashStartAddress + (_n) * (FlashPageSize))

#endif /* BDD3BB3E_4AC2_449A_B31A_278400B7E240 */
