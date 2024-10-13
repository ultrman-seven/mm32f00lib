#include "wyOstream.hpp"

#ifdef __Ostream_Use_STL
#include "stack"
#else
#include "stdio.h"
#endif

using namespace __wyOstream;

void WyOstream4MCU::putStr(char *s)
{
    while (*s)
        this->putChar(*s++);
}
void WyOstream4MCU::putStr(const char *s)
{
    while (*s)
        this->putChar(*s++);
}
void WyOstream4MCU::putBin(uint64_t val, uint8_t bitLen)
{
    uint64_t mask = 1;
    mask <<= (bitLen - 1);
    this->putStr("0b");
    while (bitLen--)
    {
        this->putChar((val & mask) ? '1' : '0');
        val <<= 1;
    }
}

void WyOstream4MCU::putHex(uint64_t val)
{
    char str[20];
    uint32_t foo;
    foo = val >> 32;
    sprintf(str, "%x", foo);
    this->putStr("0x");
    this->putStr(str);
    this->putChar(' ');
    foo = val & 0xffffffff;
    sprintf(str, "%x", foo);
    this->putStr(str);
}

void WyOstream4MCU::putInt(int val)
{

#ifdef __Ostream_Use_STL
    std::stack<char> stk;
    if (val < 0)
    {
        this->putChar('-');
        val = -val;
    }
    while (val >= 10)
    {
        stk.push((val % 10) + '0');
        val /= 10;
    }
    this->putChar(val + '0');
    while (!stk.empty())
    {
        this->putChar(stk.top());
        stk.pop();
    }
#else
    char str[20];
    sprintf(str, "%d", val);
    this->putStr(str);
#endif
}

void WyOstream4MCU::putFloat(float val, int fmt)
{
#ifdef __Ostream_Use_STL
    int intPart = val;

    this->putInt(intPart);
    val -= intPart;
    while (fmt--)
        val *= 10;
    intPart = val;
    if (intPart < 0)
        intPart = -intPart;
    this->putInt(intPart);
#else
    char str[20];
    sprintf(str, "%f", val);
    this->putStr(str);
#endif
}
WyOstream4MCU &WyOstream4MCU::operator<<(int32_t num)
{
    putInt(num);
    return *this;
}
WyOstream4MCU &WyOstream4MCU::operator<<(float num)
{
    putFloat(num, 2);
    return *this;
}
WyOstream4MCU &WyOstream4MCU::operator<<(uint8_t d)
{
    this->putChar(d);
    return *this;
}
WyOstream4MCU &WyOstream4MCU::operator<<(const char c)
{
    this->putChar(c);
    return *this;
}
WyOstream4MCU &WyOstream4MCU::operator<<(char const *s)
{
    while (*s)
        this->putChar(*s++);
    return *this;
}
WyOstream4MCU &WyOstream4MCU::operator<<(char *s)
{
    while (*s)
        this->putChar(*s++);
    return *this;
}
