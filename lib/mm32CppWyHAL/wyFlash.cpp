#include "wyFlash.hpp"
#include "cppHalReg.hpp"

using flash::__FlashBase;
using flash::Counter;
using flash::FlashArray;
using flash::paraSaver;

typedef enum
{
    FLASH_BUSY = 1,  ///< FLASH busy status
    FLASH_ERROR_PG,  ///< FLASH programming error status
    FLASH_ERROR_WRP, ///< FLASH write protection error status
    FLASH_COMPLETE,  ///< FLASH end of operation status
    FLASH_TIMEOUT    ///< FLASH Last operation timed out status
} FLASH_Status;

typedef enum
{
    FLASH_FLAG_EOP = FLASH_SR_EOP,           ///< FLASH End of Operation flag
    FLASH_FLAG_PGERR = FLASH_SR_PGERR,       ///< FLASH Program error flag
    FLASH_FLAG_WRPRTERR = FLASH_SR_WRPRTERR, ///< FLASH Write protected error flag
    FLASH_FLAG_BSY = FLASH_SR_BUSY,          ///< FLASH Busy flag
    FLASH_FLAG_OPTERR = FLASH_OBR_OPTERR     ///< FLASH Option Byte error flag
} FLASH_FLAG_TypeDef;

typedef enum
{
    FLASH_Latency_0 = FLASH_ACR_LATENCY_0, ///< FLASH Zero Latency cycle
    FLASH_Latency_1 = FLASH_ACR_LATENCY_1, ///< FLASH One Latency cycle
    FLASH_Latency_2 = FLASH_ACR_LATENCY_2, ///< FLASH Two Latency cycles
    FLASH_Latency_3 = FLASH_ACR_LATENCY_3  ///< FLASH Three Latency cycles
} FLASH_Latency_TypeDef;

#define FLASH_KEY1 ((u32)0x45670123)
#define FLASH_KEY2 ((u32)0xCDEF89AB)
inline void __FlashLock()
{
    FLASH->CR |= FLASH_CR_LOCK;
}

inline void __FlashUnlock()
{
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
}

void __FLASH_SetLatency(FLASH_Latency_TypeDef latency)
{
    FLASH->ACR = (FLASH->ACR & (~FLASH_ACR_LATENCY)) | latency;
}

void FLASH_ClearFlag(uint16_t flag)
{
    FLASH->SR = flag;
}

void __FlashEraseSector_NoWait(uint32_t address)
{
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_FLAG_EOP);

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = address;
    FLASH->CR |= FLASH_CR_STRT;
}

uint8_t FlashWaitForLastOperation(uint32_t time_out = 0xff)
{
    volatile uint32_t i;
    uint8_t ret;
    do
    {
        ret =
            ((FLASH->SR & FLASH_SR_BUSY))
                ? FLASH_BUSY
                : ((FLASH->SR & FLASH_SR_PGERR) ? FLASH_ERROR_PG
                                                : ((FLASH->SR & FLASH_SR_WRPRTERR) ? FLASH_ERROR_WRP : FLASH_COMPLETE));
        time_out--;
        for (i = 0xFF; i != 0; i--)
            ;
    } while ((ret == FLASH_BUSY) && (time_out != 0x00));

    FLASH->CR = 0;
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_WRPRTERR | FLASH_SR_PGERR;
    return ((time_out == 0x00) ? FLASH_TIMEOUT : ret);
}

uint8_t FlashProgramHalfWord(uint32_t address, uint16_t data)
{
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_FLAG_EOP);
    FLASH->CR |= FLASH_CR_PG;
    *(__IO uint16_t *)address = data;

    return FlashWaitForLastOperation(0xf);
}

__FlashBase::__FlashBase(char const *const markStr, uint32_t startAdd, const uint32_t bl)
    : mkStr(markStr), addressStart(startAdd), blockLen(bl)
{
    __FLASH_SetLatency(FLASH_Latency_1);
}

uint8_t __FlashBase::program(uint8_t *data, uint8_t dataLen)
{
    uint16_t *dataPtr = (uint16_t *)data;
    uint16_t dataTail;
    uint8_t cnt = dataLen;
    uint8_t state;

    if (dataLen & 0x01)
    {
        dataTail = data[dataLen - 1];
        dataTail <<= 8;
        --cnt;
    }

    cnt >>= 1;
    while (cnt--)
    {
        state = FlashProgramHalfWord(addressStart + addressOffset, *dataPtr++);
        addressOffset += 2;
        if (state == FLASH_TIMEOUT)
            return 0;
    }
    if (dataLen & 0x01)
    {
        state = FlashProgramHalfWord(addressStart + addressOffset, dataTail);
        addressOffset += 2;
        if (state == FLASH_TIMEOUT)
            return 0;
    }
    return 1;
}

paraSaver::paraSaver(char const *markStr, uint32_t _dataLen, uint32_t _startAdd, const uint32_t _blockLen)
    : __FlashBase(markStr, _startAdd, _blockLen), dataLen(_dataLen)
{
    if (!deflowerVirgin())
    {
        if (this->scanTill0xff(this->dataLen) == 1)
            this->resetBlock();
    }
}

#include "string.h"
uint8_t paraSaver::load(void *data)
{
    uint8_t *tmp;
    if (this->addressOffset <= this->mkStrLen)
        return 1;
    tmp = (uint8_t *)(addressStart + addressOffset - dataLen);
    memcpy(data, tmp, dataLen);
    return 0;
}

bool paraSaver::isEmpty()
{
    if (this->addressOffset <= this->mkStrLen)
        return true;
    return false;
}

uint8_t paraSaver::save(void *data)
{
    uint8_t optState;
    __FlashUnlock();

    /// TODO
    if ((addressOffset + this->dataLen) >= this->blockLen)
    {
        this->resetBlock();
    }
    optState = this->program((uint8_t *)data, this->dataLen);
    __FlashLock();
    return optState;
}

uint8_t __FlashBase::deflowerVirgin()
{
    uint8_t isVirgin = 0;
    char *flashData = (char *)addressStart;
    char const *markStr = this->mkStr;

    this->mkStrLen = 0;
    while (markStr[mkStrLen])
    {
        if (markStr[mkStrLen] != flashData[mkStrLen])
        {
            isVirgin = 1;
            break;
        }
        ++mkStrLen;
    }
    if (this->mkStrLen & 0x01)
        ++this->mkStrLen;

    this->addressOffset += mkStrLen;
    if (isVirgin)
        this->resetBlock();
    return isVirgin;
}

uint8_t __FlashBase::scanTill0xff(uint8_t dl)
{
    uint8_t offsetStep;
    uint8_t cnt;
    uint16_t *tmp;

    if (this->addressOffset >= this->blockLen)
        return 1;
    if (dl & 0x01)
        ++dl;
    offsetStep = dl;
    dl >>= 1;

    while (this->addressOffset < this->blockLen)
    {
        tmp = (uint16_t *)(this->addressStart + this->addressOffset);
        cnt = dl;

        while (cnt--)
        {
            if (*tmp++ != 0xffff)
                goto __FLASH_OffsetIncStep;
        }
        return 0;

    __FLASH_OffsetIncStep:
        this->addressOffset += offsetStep;
    }

    return 1;
}

void __FlashBase::resetBlock(void)
{
    __FlashUnlock();
    __FlashEraseSector_NoWait(this->addressStart);
    this->mkStrLen = strlen(this->mkStr);
    if (this->mkStrLen & 0x01)
        ++this->mkStrLen;
    this->addressOffset = 0;
    FlashWaitForLastOperation(0xfff);
    this->program((uint8_t *)mkStr, mkStrLen);
    __FlashLock();
}

FlashArray::FlashArray(const char *mark, uint8_t us, uint32_t startAdd, const uint32_t blockLen)
    : __FlashBase(mark, startAdd, blockLen), unitSize(us)
{
    if (!deflowerVirgin())
    {
        if (this->scanTill0xff(this->unitSize) == 1)
            this->resetBlock();
    }
    this->arrayLen = this->addressOffset - this->mkStrLen;
    this->arrayLen /= this->unitSize;
}

bool FlashArray::append(uint8_t *dat)
{
    bool err = false;
    if (this->addressOffset + this->unitSize >= this->blockLen)
    {
        clearAll();
        err = true;
    }
    __FlashUnlock();
    this->program(dat, unitSize);
    __FlashLock();
    ++this->arrayLen;
    return err;
}

void FlashArray::clearAll(void)
{
    this->resetBlock();
    this->arrayLen = 0;
}

bool FlashArray::searchData(uint8_t *data)
{
    uint32_t cnt = this->arrayLen;
    uint32_t add = this->addressStart + this->mkStrLen;
    while (cnt--)
    {
        if (memcmp(data, (void *)add, this->unitSize) == 0)
            return true;
        add += this->unitSize;
    }
    return false;
}

Counter::Counter(const char *mark, uint32_t addressStart, uint32_t BlockLen)
    : __FlashBase(mark, addressStart, BlockLen)
{
    uint8_t cnt4mark;
    uint16_t tmp;
    if (!deflowerVirgin())
    {
        if (this->scanTill0xff(2) == 1)
            this->resetBlock();
    }
    if (this->addressOffset <= this->mkStrLen)
    {
        cnt = 0;
        this->markVal = 0;
        return;
    }

    memcpy(&this->markVal, (void *)(this->addressStart + this->addressOffset - 2), 2);
    cnt = this->addressOffset - this->mkStrLen - 2;
    cnt *= 8;

    tmp = this->markVal;
    if (0 == tmp)
    {
        this->markVal = 0;
        this->cnt += 16;
        return;
    }
    while (0 == (tmp & 0x01))
    {
        ++cnt;
        tmp >>= 1;
    }
}

void Counter::add()
{
    if (this->markVal == 0)
    {
        this->markVal = 0xfffe;
        // if(this.)
        this->addressOffset += 2;
    }
}

void Counter::reset()
{
    this->cnt = 0;
    this->resetBlock();
}

uint32_t Counter::operator++() //++a
{
    this->add();
    return this->cnt;
}

uint32_t Counter::operator++(int) // a++
{
    uint32_t cntLast;
    cntLast = this->cnt;
    this->add();
    return cntLast;
}

uint32_t Counter::getCount() { return this->cnt; }
