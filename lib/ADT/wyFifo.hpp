#ifndef F2471CBB_8748_4558_9C35_6C04E95D27FB
#define F2471CBB_8748_4558_9C35_6C04E95D27FB
#include "stdint.h"

namespace adt
{
    template <typename FIFO_valType, uint32_t _bufLen = 0>
    class FIFO
    {
    private:
        FIFO_valType _buf[_bufLen];
        FIFO_valType *buf;
        uint32_t bufLen;
        uint32_t idxFront;
        uint32_t dataLen;

    public:
        FIFO() : idxFront(0), dataLen(0), buf(_buf), bufLen(_bufLen) {}
        FIFO(FIFO_valType *bf, uint32_t dl) : idxFront(0), dataLen(0), buf(bf), bufLen(dl) {}
        void push(FIFO_valType &d)
        {
            if (this->idxFront + this->dataLen >= bufLen)
                this->buf[this->idxFront + this->dataLen - bufLen] = d;
            else
                this->buf[this->idxFront + this->dataLen] = d;
            ++this->dataLen;
            if (this->dataLen > bufLen)
                this->pop();
        }
        FIFO_valType &top(void) { return buf[idxFront]; }
        FIFO_valType &pop(void)
        {
            if (!this->dataLen)
                return buf[0];
            --this->dataLen;
            if (this->idxFront + 1 < bufLen)
                return this->buf[this->idxFront++];
            this->idxFront = 0;
            return this->buf[bufLen - 1];
        }
        uint32_t getDataLen(void) { return this->dataLen; }
        // ~FIFO();
    };

} // namespace adt

#endif /* F2471CBB_8748_4558_9C35_6C04E95D27FB */
