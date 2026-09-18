#include "wyI2c.hpp"
#include "reg_common.h"
#include "core_cm0.h"
#include "cppHalReg.hpp"

using namespace IIC;
using namespace GPIO;

static IIC_HardwareMasterObject *_i2cObj[1];

IIC_Object::IIC_Object(const char *sclPin, const char *sdaPin, GPIO::Mode mScl, GPIO::Mode mSda)
    : scl(GpioPin(sclPin, mScl)), sda(GpioPin(sdaPin, mSda))
{
}

#define I2cRegCr_RestartEn 0x0400
#define I2cRegCr_StopEn 0x0200
#define I2cRegCr_EmpInt 0x0100
#define I2cRegCr_StopInt 0x0080
#define I2cRegCr_SlaveDisable 0x0040
#define I2cRegCr_RepEn 0x0020
// #define I2cRegCr_RepEn 0x0000
#define I2cRegCr_MasterAdd10 0x0010
#define I2cRegCr_SlaveAdd10 0x0008
#define I2cRegCr_SpeedFast 0x0004
#define I2cRegCr_SpeedStd 0x0002
#define I2cRegCr_MasterEn 0x0001

#define I2cIt_Tx 0x0010
#define I2cIt_TxEmpty 0x0010
#define I2cIt_TxAbort 0x0040
#define I2cIt_StopDet 0x0200
#define I2cIt_StartDet 0x0400

IIC_HardwareMasterObject::IIC_HardwareMasterObject(uint8_t channel, const char *scl, const char *sda, uint16_t clkHighCnt, uint16_t clkLowCnt, uint8_t fastMode)
{
    uint16_t cr;
    uint8_t af = 0xff;
    uint8_t ch = channel - 1;
    this->i2c = I2C1;
    // RCC
    RCC->APB1ENR |= (RCC_APB1ENR_I2C1);
    this->i2c->ENR &= 0xfffe;
    // _i2cObj[0] = (void*)this;
    _i2cObj[ch] = this;

    // AF
    af = GPIO::afTable2afVal(scl, __I2C_Scl_GPIO_AFs[ch]);
    if (af != 0xff)
        GPIO::afConfig(scl, af, GPIO::Mode_AF_OD_Floating);
    af = GPIO::afTable2afVal(sda, __I2C_Sda_GPIO_AFs[ch]);
    if (af != 0xff)
        GPIO::afConfig(sda, af, GPIO::Mode_AF_OD_Floating);
    this->needStop = 1;
    this->txLen = 0;
    if (fastMode)
    {
        cr = I2cRegCr_RepEn | I2cRegCr_SpeedFast | I2cRegCr_MasterEn | I2cRegCr_SlaveDisable | I2C_CR_EMPINT;
        this->i2c->CR = cr;
        this->i2c->FSHR = clkHighCnt;
        this->i2c->FSLR = clkLowCnt;
    }
    else
    {
        cr = I2cRegCr_RepEn | I2cRegCr_SpeedStd | I2cRegCr_MasterEn | I2cRegCr_SlaveDisable | I2C_CR_EMPINT;
        this->i2c->CR = cr;
        this->i2c->SSHR = clkHighCnt;
        this->i2c->SSLR = clkLowCnt;
    }
    i2c->IC_INTR_MASK &= 0xc000;
    i2c->IC_RX_TL = 0x00;
    // i2c->IC_TX_TL = 1;
    i2c->IC_TX_TL = 0;
    // this->i2c->IC_INTR_MASK = 0;
    // i2c->IC_INTR_MASK |= (I2cIt_StartDet | I2cIt_StopDet);
    i2c->IC_INTR_MASK |= (I2cIt_StopDet);

    NVIC_SetPriority(I2C1_IRQn, 2);
    NVIC_EnableIRQ(I2C1_IRQn);
}

uint8_t IIC_HardwareMasterObject::sendAsync(uint8_t add, uint8_t *datPtr, uint32_t len, uint8_t needStop, void (*cbk)(void *), void *arg)
{
    this->arg = arg;
    this->cbk = cbk;
    this->txBuf = datPtr;
    this->txLen = len;

    if (this->needStop)
    {
        this->i2c->ENR &= 0xfffe;
        // this->i2c->CR |= I2cRegCr_RepEn;
        this->i2c->TAR = add;
        this->i2c->ENR |= 0x01;
        // this->i2c->DR = (datPtr[0]);
    }
    this->needStop = needStop;
    this->i2c->IC_INTR_MASK |= (I2cIt_TxEmpty);
    return 0;
}
void IIC::_irqHandle(IIC_HardwareMasterObject *handle)
{
    // if (handle->i2c->ISR & I2cIt_StartDet)
    // {
    //     handle->i2c->IC_CLR_START_DET;
    // }
    if (handle->i2c->ISR & I2cIt_TxEmpty)
    {
        if (handle->txLen)
        {
            uint8_t tx;
            tx = *((handle->txBuf)++);
            if ((handle->needStop) && (1 == handle->txLen))
            {
                handle->i2c->CR |= I2cRegCr_StopEn;
            }
            --(handle->txLen);
            handle->i2c->DR = tx;
        }
        else
        {
            handle->i2c->IC_INTR_MASK &= (uint16_t)(~I2cIt_TxEmpty);
            // handle->i2c->IC_INTR_MASK &= (uint16_t)(0xDFEF);
            if (!(handle->needStop))
            {
                if (handle->cbk)
                    handle->cbk(handle->arg);
            }
        }
    }

    // if ((handle->i2c->ISR & I2cIt_TxAbort))
    if ((handle->i2c->ISR & I2cIt_StopDet))
    {
        volatile uint16_t foo;
        foo = (handle->i2c->IC_CLR_STOP_DET) & I2C_TX_ABRT;
        handle->i2c->CR &= (uint16_t)(~(I2cRegCr_StopEn));

        handle->i2c->ENR &= 0xfffe;
        if (handle->cbk)
            handle->cbk(handle->arg);
    }
}

extern "C"
{
    void I2C1_IRQHandler(void)
    {
        _irqHandle(_i2cObj[0]);
    }
}

static inline void __iicDelay(void)
{
    __IO uint32_t time = 3;
    while (time--)
        __NOP();
}

inline void IIC_Object::ack(uint8_t ys)
{
    scl = 0;
    // sda.setMode(Mode_IN_FLOATING);
    sda = !ys;
    __iicDelay();
    scl = 1;
    __iicDelay();
    scl = 0;
    // sda = 1;
}

inline bool IIC_Object::waitAck(void)
{
    uint8_t waitTime = 0xff;

    // sda.setMode(Mode_Out_PP);
    sda = 1;
    __iicDelay();
    scl = 1;
    __iicDelay();
    while (sda.read())
    {
        if (waitTime--)
        {
            stop();
            return false;
        }
        __iicDelay();
    }
    scl = 0;
    return true;
}

inline bool IIC_Object::start(void)
{
    // sda.setMode(Mode_Out_PP);
    sda = 1;
    if (!sda.read())
        return false;
    scl = 1;
    __iicDelay();
    sda = 0;
    if (sda.read())
        return false;
    __iicDelay();
    scl = 0;
    return true;
}

inline void IIC_Object::stop(void)
{
    // sda.setMode(Mode_Out_PP);
    scl = 0;
    sda = 0;
    __iicDelay();
    scl = 1;
    sda = 1;
    __iicDelay();
}
inline void IIC_Object::sendByte(uint8_t dtx)
{
    uint8_t cnt = 8;
    scl = 0;
    while (cnt--)
    {
        sda = (dtx & 0x80);
        dtx <<= 1;

        __iicDelay();
        scl = 1;
        __iicDelay();
        scl = 0;
        __iicDelay();
    }
}

inline uint8_t IIC_Object::readByte(void)
{
    uint8_t cnt = 8;
    uint8_t drx = 0;
    sda = 1;
    while (cnt--)
    {
        scl = 0;
        __iicDelay();
        scl = 1;
        drx <<= 1;
        if (sda.read())
            ++drx;
        __iicDelay();
    }
    return drx;
}

uint8_t IIC_Object::sendSameByte(uint8_t add, uint8_t reg, uint8_t len, uint8_t txData)
{
    if (!this->start())
        return 1;
    this->sendByte(add << 1);
    if (!this->waitAck())
    {
        this->stop();
        return 1;
    }
    this->sendByte(reg);
    this->waitAck();
    while (len--)
    {
        this->sendByte(txData);
        if (!this->waitAck())
        {
            this->stop();
            return 1;
        }
    }
    this->stop();
    return 0;
}

uint8_t IIC_Object::send(uint8_t add, uint8_t *txData, uint32_t len, uint8_t start, uint8_t stop)
{
    if (start)
    {
        if (!this->start())
            return 1;
        this->sendByte(add << 1);
        if (!this->waitAck())
        {
            this->stop();
            return 1;
        }
    }
    while (len--)
    {
        this->sendByte(*txData++);
        if (!this->waitAck())
        {
            this->stop();
            return 1;
        }
    }
    if (stop)
        this->stop();
    return 0;
}

uint8_t IIC_Object::send(uint8_t add, uint8_t reg, uint8_t len, uint8_t *txData)
{
    if (!this->start())
        return 1;
    this->sendByte(add << 1);
    if (!this->waitAck())
    {
        this->stop();
        return 1;
    }
    this->sendByte(reg);
    this->waitAck();
    while (len--)
    {
        this->sendByte(*txData++);
        if (!this->waitAck())
        {
            this->stop();
            return 1;
        }
    }
    this->stop();
    return 0;
}
uint8_t IIC_Object::send16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *txData)
{
    uint8_t *regPtr = (uint8_t *)&reg;
    if (!this->start())
        return 1;
    this->sendByte(add << 1);
    if (!this->waitAck())
    {
        this->stop();
        return 1;
    }
    this->sendByte(regPtr[1]);
    this->waitAck();
    this->sendByte(regPtr[0]);
    this->waitAck();
    while (len--)
    {
        this->sendByte(*txData++);
        if (!this->waitAck())
        {
            this->stop();
            return 1;
        }
    }
    this->stop();
    return 0;
}

uint8_t IIC_Object::read(uint8_t add, uint8_t reg, uint8_t len, uint8_t *rxData)
{
    if (!this->start())
        return 1;
    this->sendByte(add << 1);
    if (!this->waitAck())
    {
        this->stop();
        return 1;
    }
    this->sendByte(reg);
    this->waitAck();

    this->start();
    this->sendByte((add << 1) + 1);
    this->waitAck();
    while (len--)
    {
        *rxData = this->readByte();
        this->ack(len);
        ++rxData;
    }
    this->stop();
    return 0;
}

uint8_t IIC_Object::read16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *rxData)
{
    uint8_t *regPtr = (uint8_t *)&reg;
    if (!this->start())
        return 1;
    this->sendByte(add << 1);
    if (!this->waitAck())
    {
        this->stop();
        return 1;
    }
    this->sendByte(regPtr[1]);
    this->waitAck();
    this->sendByte(regPtr[0]);
    this->waitAck();

    this->start();
    this->sendByte((add << 1) + 1);
    this->waitAck();
    while (len--)
    {
        *rxData = this->readByte();
        this->ack(len);
        ++rxData;
    }
    this->stop();
    return 0;
}
