#ifndef C2342E4A_0F6A_4A86_A696_31EBC200D0B3
#define C2342E4A_0F6A_4A86_A696_31EBC200D0B3
#include "stdint.h"

namespace __wyIstream
{
    class FIFO
    {
    private:
        uint8_t *dataBuf;
        uint32_t dataLen;
        uint32_t bufLen;

        uint32_t idxFront;

    public:
        void setFifoBuf(uint8_t *buf, uint32_t len);
        void push(uint8_t d);
        uint8_t pop(void);
        uint32_t getDataLen(void);
        uint8_t operator[](int32_t);
        void byteProcess(uint8_t);
        void clear();
    };

    class CMD_Listener
    {
    private:
        struct waitKeyWord
        {
            char const *password;
            // bool flag;
            // uint8_t len;
            uint32_t flag : 1;
            uint32_t len : 31;
        };
        inline bool checkKeyWord(waitKeyWord *w);

        waitKeyWord start;
        waitKeyWord end;
        FIFO sFifo;
        // FIFO dFifo;
        uint8_t *datBuf;
        uint32_t datLen;
        // uint32_t datCnt = 0;
        void (**cmdF)(uint8_t *, uint32_t) = nullptr;
        uint8_t cmdLen = 0;
        inline void reset();

    public:
        void byteProcess(uint8_t);
        void setKeyWord(char const *s = nullptr, char const *e = nullptr);
        void setBuf(uint8_t *buf, uint32_t len);
        void addCmd(void (**callbacks)(uint8_t *, uint32_t), uint8_t l);
        void addCmd(void (*f)(uint8_t *, uint32_t));
    };

    class WyIstream4MCU
    {
    protected:
        CMD_Listener cmd;
        FIFO fifo;

    public:
        // WyIstream4MCU(/* args */);
        // ~WyIstream4MCU();

        /// @brief readBuff
        ///
        /// @param d to store data that u want to read
        /// @return true: \n
        ///  there is data in FIFO and u read data successfully. \n
        /// @return false: \n
        ///  no data in FIFO and u get nothing.
        bool readBuff(uint8_t &d);
        bool readBuff(uint8_t *d, uint8_t &len);
        uint32_t getBufDataLen(void);

        void setTrigger(char const *start, char const *stop);
        void addCMD(void (**f)(uint8_t *, uint32_t), uint32_t len);
        void addCMD(void (*f)(uint8_t *, uint32_t));

        WyIstream4MCU &operator>>(uint8_t &d);
    };

}

#endif /* C2342E4A_0F6A_4A86_A696_31EBC200D0B3 */
