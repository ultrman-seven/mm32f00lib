#include "wyFlash.hpp"
#include "cppHalReg.hpp"
// #include "reg_flash.h"

using flash::FlashBlock;

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

void FLASH_ClearFlag(uint16_t flag)
{
    FLASH->SR = flag;
}
// #include "wySys.hpp"
uint8_t FlashWaitForLastOperation(uint32_t time_out = 0xff)
{
    volatile uint32_t i;
    uint8_t ret;
    // sys::delayMs(5);
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

uint8_t FlashBlock::program(uint8_t *data, uint8_t dataLen)
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

typedef enum
{
    FLASH_Latency_0 = FLASH_ACR_LATENCY_0, ///< FLASH Zero Latency cycle
    FLASH_Latency_1 = FLASH_ACR_LATENCY_1, ///< FLASH One Latency cycle
    FLASH_Latency_2 = FLASH_ACR_LATENCY_2, ///< FLASH Two Latency cycles
    FLASH_Latency_3 = FLASH_ACR_LATENCY_3  ///< FLASH Three Latency cycles
} FLASH_Latency_TypeDef;

void FLASH_SetLatency(FLASH_Latency_TypeDef latency)
{
    FLASH->ACR = (FLASH->ACR & (~FLASH_ACR_LATENCY)) | latency;
}

FlashBlock::FlashBlock(char const *markStr, uint32_t _dataLen, uint32_t _startAdd, const uint32_t _blockLen)
    : addressStart(_startAdd), mkStr(markStr), blockLen(_blockLen), dataLen(_dataLen)
{
    FLASH_SetLatency(FLASH_Latency_1);
    deflowerVirgin();
}

#include "string.h"
uint8_t FlashBlock::load(void *data)
{
    uint8_t *tmp;
    if (this->addressOffset <= this->mkStrLen)
        return 1;
    tmp = (uint8_t *)(addressStart + addressOffset - dataLen);
    memcpy(data, tmp, dataLen);
    return 0;
}

bool FlashBlock::isEmpty()
{
    if (this->addressOffset <= this->mkStrLen)
        return true;
    return false;
}

uint8_t FlashBlock::save(void *data)
{
    uint8_t optState;
    this->unlock();

    /// TODO
    if ((addressOffset + this->dataLen) >= this->blockLen)
    {
        this->resetBlock();
    }
    optState = this->program((uint8_t *)data, this->dataLen);
    this->lock();
    return optState;
}

#define FLASH_KEY1 ((u32)0x45670123)
#define FLASH_KEY2 ((u32)0xCDEF89AB)

void FlashBlock::lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}
void FlashBlock::unlock(void)
{
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
}

uint8_t FlashBlock::deflowerVirgin()
{
    uint8_t isVirgin = 0;
    char *flashData = (char *)addressStart;
    char const *markStr = this->mkStr;

    // FLASH_SetLatency(FLASH_Latency_1);

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
    else
    {
        if (this->scanTill0xff() == 1)
            this->resetBlock();
    }

    return isVirgin;
}

uint8_t FlashBlock::scanTill0xff()
{
    uint8_t dl = dataLen;
    uint8_t offsetStep;
    uint8_t cnt;
    uint16_t *tmp;

    if (this->addressOffset >= this->blockLen)
        return 1;
    if (dl & 0x01)
        ++dl;
    offsetStep = dl;
    dl >>= 1;

    /// TODO: need to test
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

    // if (this->addressOffset >= this->blockLen)
    //     return 1;
    // tmp = (uint16_t *)(this->addressStart + this->addressOffset);

    // while (this->addressOffset < this->blockLen)
    // {
    //     if (*tmp++ == 0xffff)
    //         return 0;
    //     this->addressOffset += 2;
    // }

    // return 1;
}

void FlashBlock::eraseSector()
{
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_FLAG_EOP);

    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = addressStart;
    FLASH->CR |= FLASH_CR_STRT;
    FlashWaitForLastOperation(0xfff);
}

void FlashBlock::resetBlock(void)
{
    this->mkStrLen = strlen(this->mkStr);

    this->unlock();
    this->eraseSector();
    this->lock();

    this->addressOffset = 0;
    this->unlock();
    this->program((uint8_t *)mkStr, mkStrLen);
    this->lock();

    if (this->mkStrLen & 0x01)
        ++this->mkStrLen;
}
