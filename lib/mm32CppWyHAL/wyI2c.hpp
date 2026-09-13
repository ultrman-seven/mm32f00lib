#ifndef __MM32_CPP_HAL_WY_LIB_IIC_HPP__
#define __MM32_CPP_HAL_WY_LIB_IIC_HPP__

#include "wyGpio.hpp"
#include "reg_i2c.h"

namespace IIC
{
    class IIC_Object
    {
    private:
        GPIO::GpioPin scl;
        GPIO::GpioPin sda;

        inline bool start(void);
        inline void stop(void);
        inline bool waitAck(void);
        inline void ack(uint8_t);
        inline void sendByte(uint8_t);
        inline uint8_t readByte(void);

    public:
        // IIC_Object(const char *scl, const char *sda, bool exPullUp = true);
        IIC_Object(const char *scl, const char *sda, GPIO::Mode mScl = GPIO::Mode_Out_PP, GPIO::Mode mSda = GPIO::Mode_Out_PP);
        uint8_t send(uint8_t add, uint8_t *txData, uint32_t len, uint8_t start, uint8_t stop);
        uint8_t send(uint8_t add, uint8_t reg, uint8_t len, uint8_t *txData);
        uint8_t sendSameByte(uint8_t add, uint8_t reg, uint8_t len, uint8_t txData);
        uint8_t read(uint8_t add, uint8_t reg, uint8_t len, uint8_t *rxData);
        uint8_t read16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *rxData);
        uint8_t send16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *txData);
    };

    class IIC_HardwareMasterObject
    {
    private:
        I2C_TypeDef *i2c;
        void (*cbk)(void *);
        void *arg;
        uint32_t txLen;
        uint8_t *txBuf;
        uint8_t needStop;

        friend void _irqHandle(IIC_HardwareMasterObject *);

    public:
        IIC_HardwareMasterObject(){}
        IIC_HardwareMasterObject(uint8_t channel, const char *scl, const char *sda, uint16_t clkHighCnt, uint16_t clkLowCnt, uint8_t mode);
        // void init(uint8_t channel, const char *scl, const char *sda, uint16_t clkHighCnt, uint16_t clkLowCnt, uint8_t mode);
        uint8_t sendAsync(uint8_t add, uint8_t *datPtr, uint32_t len, uint8_t needStop, void (*cbk)(void *), void *arg);
        // ~IIC_HardwareMasterObject();
    };

}

#endif /* __MM32_CPP_HAL_WY_LIB_IIC_HPP__ */
