#ifndef __MM32_CPP_HAL_WY_LIB_UART_HPP__
#define __MM32_CPP_HAL_WY_LIB_UART_HPP__

#include "wyOstream.hpp"
#include "reg_uart.h"
namespace UART
{
    class Serial// : public __wyOstream::WyOstream4MCU
    {
    private:
        uint8_t num;
        UART_TypeDef *uart;

    public:
        Serial(uint8_t n, const char *tx, const char *rx=nullptr, uint32_t baud=115200);
        ~Serial();

        void sendByte(uint8_t dat);
        void sendByte(uint8_t *dat, uint8_t len);

        // bool txComplete(void);
        // bool received(void);
        // uint8_t receiveByte(void);

        // /// @brief enable interrupt
        // /// @param f if callback is given, u deal with rx data whatever u want.
        // /// Otherwise rx data will de stored in fifo
        // void setInterrupt(void (*f)(uint8_t) = nullptr);
        // // uint16_t getBuffLen(void);
        // // uint8_t readBuff(void);

        // /// @brief readBuff
        // ///
        // /// @param d to store data that u want to read
        // /// @return true: \n
        // ///  there is data in FIFO and u read data successfully. \n
        // /// @return false: \n
        // ///  no data in FIFO and u get nothing.
        // bool readBuff(uint8_t &d);
        // bool readBuff(uint8_t *d, uint8_t &len);

        // /// @brief setStartTrigger
        // /// @param d
        // /// @param l
        // void setStartTrigger(uint8_t *d, uint8_t l);
        // /// @brief setStopTrigger
        // /// @param d
        // /// @param l
        // void setStopTrigger(uint8_t *d, uint8_t l);

        // void setDMA(uint8_t *add, uint8_t len);

        // virtual void putChar(uint8_t c);
        // Serial &operator>>(uint8_t &dat);


        Serial &operator<<(uint8_t dat);
        Serial &operator<<(const char dat);
        Serial &operator<<(int32_t num);
        Serial &operator<<(float num);
        Serial &operator<<(const char *s);
        Serial &operator<<(char *s);
    };

} // namespace UART

#endif /* __MM32_CPP_HAL_WY_LIB_UART_HPP__ */
