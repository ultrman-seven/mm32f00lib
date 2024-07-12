#include "wyI2c.hpp"

using namespace IIC;
using namespace GPIO;
IIC_Object::IIC_Object(const char *sclPin, const char *sdaPin, GPIO::Mode mScl, GPIO::Mode mSda)
    : scl(GpioPin(sclPin, mScl)), sda(GpioPin(sdaPin, mSda))
{
}
IIC_HardwareObject::IIC_HardwareObject(/* args */)
{
}
#include "reg_common.h"
#include "core_cm0.h"
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
    scl = 1;
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
    }
    this->stop();
    return 0;
}
