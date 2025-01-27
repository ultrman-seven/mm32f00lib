#include "wyOstream.hpp"

#define __Ostream_Use_Private_STL

#ifdef __Ostream_Use_STL
#include "stack"
// #elif defined __Ostream_Use_Private_STL

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
#elif defined __Ostream_Use_Private_STL
    char stk[20];
    uint8_t cnt = 0;

    if (val < 0)
    {
        this->putChar('-');
        val = -val;
    }
    while (val >= 10)
    {
        stk[cnt++] = (val % 10) + '0';
        val /= 10;
    }
    this->putChar(val + '0');
    while (cnt--)
    {
        this->putChar(stk[cnt]);
    }
#else
    // char str[20];
    // sprintf(str, "%d", val);
    // this->putStr(str);
#endif
}

void WyOstream4MCU::putFloat(float val, int fmt)
{
#if (defined __Ostream_Use_STL) || (defined __Ostream_Use_Private_STL)

    int intPart;
    if (val < 0)
    {
        val = -val;
        this->putChar('-');
    }
    intPart = val;
    this->putInt(intPart);
    val -= intPart;
    this->putChar('.');
    while (fmt--)
    {
        val *= 10;
        if (val < 1)
            this->putChar('0');
    }
    intPart = val;
    if (intPart)
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
