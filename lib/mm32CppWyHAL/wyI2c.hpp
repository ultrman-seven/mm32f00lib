#ifndef __MM32_CPP_HAL_WY_LIB_IIC_HPP__
#define __MM32_CPP_HAL_WY_LIB_IIC_HPP__

#include "wyGpio.hpp"

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
        uint8_t send(uint8_t add, uint8_t reg, uint8_t len, uint8_t *txData);
        uint8_t read(uint8_t add, uint8_t reg, uint8_t len, uint8_t *rxData);
        uint8_t read16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *rxData);
        uint8_t send16bitReg(uint8_t add, uint16_t reg, uint8_t len, uint8_t *txData);
    };

    class IIC_HardwareObject
    {
    private:
        /* data */
    public:
        IIC_HardwareObject(uint8_t num);
        // ~IIC_HardwareObject();
    };

}

#endif /* __MM32_CPP_HAL_WY_LIB_IIC_HPP__ */
