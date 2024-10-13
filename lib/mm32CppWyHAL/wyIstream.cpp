#include "wyIstream.hpp"
#include "string.h"

using namespace __wyIstream;

void WyIstream4MCU::setTrigger(char const *start, char const *stop) { cmd.setKeyWord(start, stop); }
void WyIstream4MCU::addCMD(void (**f)(uint8_t *, uint32_t), uint32_t len) { cmd.addCmd(f, len); }
void WyIstream4MCU::addCMD(void (*f)(uint8_t *, uint32_t)) { cmd.addCmd(f); }
uint32_t WyIstream4MCU::getBufDataLen(void) { return fifo.getDataLen(); }

void FIFO::setFifoBuf(uint8_t *buf, uint32_t len)
{
    this->dataBuf = buf;
    this->bufLen = len;
    this->idxFront = 0;
    this->dataLen = 0;
}

void FIFO::push(uint8_t d)
{
    if (this->idxFront + this->dataLen >= this->bufLen)
        this->dataBuf[this->idxFront + this->dataLen - this->bufLen] = d;
    else
        this->dataBuf[this->idxFront + this->dataLen] = d;
    ++this->dataLen;
    if (this->dataLen > this->bufLen)
        this->pop();
}

uint8_t FIFO::pop(void)
{
    if (!this->dataLen)
        return 0;
    --this->dataLen;
    if (this->idxFront + 1 < this->bufLen)
        return this->dataBuf[this->idxFront++];
    this->idxFront = 0;
    return this->dataBuf[this->bufLen - 1];
}

uint32_t FIFO::getDataLen(void) { return this->dataLen; }

uint8_t FIFO::operator[](int32_t idx)
{
    int cmp = this->idxFront;
    idx += this->idxFront;
    while (idx < cmp)
        idx += this->dataLen;
    while (idx >= (this->idxFront + this->dataLen))
        idx -= this->dataLen;

    while (idx >= this->bufLen)
        idx -= this->bufLen;
    return this->dataBuf[idx];
}

void FIFO::byteProcess(uint8_t d)
{
    this->push(d);
}
void FIFO::clear()
{
    this->idxFront = 0;
    this->dataLen = 0;
}

void CMD_Listener::setKeyWord(char const *s, char const *e)
{
    start.password = (nullptr == s) ? "" : s;
    end.password = (nullptr == e) ? "" : e;

    start.flag = end.flag = 0;
    start.len = strlen(start.password);
    end.len = strlen(end.password);
}

void CMD_Listener::addCmd(void (*f)(uint8_t *, uint32_t))
{
    if (this->datLen <= 4)
        return;
    ++this->cmdLen;
    this->datLen -= 4;
    if (nullptr == this->cmdF)
        this->cmdF = (void (**)(uint8_t *, uint32_t))this->datBuf;
    this->datBuf += 4;
    sFifo.setFifoBuf(this->datBuf, this->datLen);
    this->reset();
}

void CMD_Listener::addCmd(void (**callbacks)(uint8_t *, uint32_t), uint8_t l)
{
    this->cmdF = callbacks;
    this->cmdLen = l;
}

inline void CMD_Listener::reset()
{
    sFifo.clear();
    this->start.flag = this->end.flag = 0;
}

void CMD_Listener::setBuf(uint8_t *buf, uint32_t len)
{
    datBuf = buf;
    datLen = len;
    sFifo.setFifoBuf(buf, len);
}

inline bool CMD_Listener::checkKeyWord(waitKeyWord *w)
{
    int cnt;
    if (sFifo.getDataLen() >= w->len)
    {
        int32_t idx4fifo = -1;
        cnt = w->len;
        while (cnt--)
        {
            if (sFifo[idx4fifo] != w->password[cnt])
                return false;
            --idx4fifo;
        }
        return true;
    }
    return false;
}

void CMD_Listener::byteProcess(uint8_t d)
{
    sFifo.push(d);
    if (0 == start.flag)
    {
        if (checkKeyWord(&this->start))
        {
            start.flag = 1;
            sFifo.clear();
            return;
        }
        return;
    }

    if (checkKeyWord(&this->end))
    {
        uint8_t cmdIdx = datBuf[0];
        uint32_t argc;
        argc = sFifo.getDataLen() - end.len - 1;
        if (cmdIdx <= cmdLen)
            cmdF[cmdIdx](datBuf + 1, argc);
        reset();
    }
    if (sFifo.getDataLen() == datLen)
        reset();
}
