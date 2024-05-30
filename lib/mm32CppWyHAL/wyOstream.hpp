#ifndef __MM32_CPP_HAL_WY_LIB_OSTREAM_HPP__
#define __MM32_CPP_HAL_WY_LIB_OSTREAM_HPP__
#include "stdint.h"
namespace __wyOstream
{
    class WyOstream4MCU
    {
    private:
        // char str[10];

    public:
        // WyOstream4MCU() = delete;
        virtual void putChar(uint8_t) = 0;
        void putStr(char *s);
        void putStr(const char *s);
        void putInt(int);
        void putFloat(float val,int fmt);

        WyOstream4MCU &operator<<(uint8_t dat);
        WyOstream4MCU &operator<<(const char dat);
        WyOstream4MCU &operator<<(int32_t num);
        WyOstream4MCU &operator<<(float num);
        WyOstream4MCU &operator<<(const char *s);
        WyOstream4MCU &operator<<(char *s);
    };
}
#endif /* __MM32_CPP_HAL_WY_LIB_OSTREAM_HPP__ */
