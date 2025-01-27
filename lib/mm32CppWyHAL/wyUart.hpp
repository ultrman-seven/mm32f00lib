#ifndef __MM32_CPP_HAL_WY_LIB_UART_HPP__
#define __MM32_CPP_HAL_WY_LIB_UART_HPP__

#include "wyOstream.hpp"
#include "wyIstream.hpp"
#include "reg_uart.h"
namespace UART
{
    class Serial : public __wyOstream::WyOstream4MCU, public __wyIstream::WyIstream4MCU
    {
    private:
        uint8_t irqMode;
        uint8_t num;
        UART_TypeDef *uart;
        void (*IRQ_Callback)(uint8_t);
        inline void nvicCfg();
        // __wyIstream::FIFO fifo;
        // __wyIstream::CMD_Listener cmd;

    public:
        Serial(uint8_t n, const char *tx, const char *rx = nullptr, uint32_t baud = 115200);
        // ~Serial();
        // __wyIstream::CMD_Listener cmdListener;
        void sendByte(uint8_t dat);
        void sendByte(uint8_t *dat, uint8_t len);
        void iqrHandler();
        // bool received(void);
        // uint8_t receiveByte(void);

        // // uint16_t getBuffLen(void);
        // // uint8_t readBuff(void);

        void setInterrupt(uint8_t *buf, uint32_t bufSize, char const *start, char const *end, void (*f)(uint8_t *, uint32_t) = nullptr);
        void setInterrupt(uint8_t *buf, uint32_t bufSize, char const *start, char const *end, uint8_t *argvBuf, uint32_t argvBufLen, uint32_t *argcPtr);
        void setInterrupt(uint8_t *buf, uint32_t bufSize);
        void setInterrupt(void (*callback)(uint8_t));
        void interruptCMD(bool);

        // void setDMA(uint8_t *add, uint8_t len);

        // virtual void putChar(uint8_t c);
        // Serial &operator>>(uint8_t &dat);

        virtual void putChar(uint8_t);
    };

} // namespace UART

#endif /* __MM32_CPP_HAL_WY_LIB_UART_HPP__ */
