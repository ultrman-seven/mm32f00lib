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
            uint32_t flag : 1;
            uint32_t len : 31;
        };
        inline bool checkKeyWord(waitKeyWord *w);

        waitKeyWord start;
        waitKeyWord end;
        FIFO sFifo;
        uint8_t *datBuf = nullptr;
        uint32_t datLen = 0;
        uint8_t *argvBuf = nullptr;
        uint32_t argvBufLen = 0;
        uint32_t *argcPtr = nullptr;
        uint8_t triggered;
        // void (**cmdF)(uint8_t *, uint32_t) = nullptr;
        void (*cmdF)(uint8_t *, uint32_t) = nullptr;
        // uint8_t cmdLen = 0;
        inline void reset();

    public:
        void byteProcess(uint8_t);
        void setKeyWord(char const *s = nullptr, char const *e = nullptr);
        void setBuf(uint8_t *buf, uint32_t len);
        void setArgvArgcBuff(uint8_t *buf, uint32_t bl, uint32_t *cp);
        bool checkTriggerState();
        // void addCmd(void (**callbacks)(uint8_t *, uint32_t), uint8_t l);
        /// @brief Data format: start sign : x, data length : 1, data : n(0-255), end sign : y
        /// @param f
        void addCmd(void (*f)(uint8_t *, uint32_t));
    };

    class WyIstream4MCU
    {
    protected:
        CMD_Listener cmd;
        FIFO fifo;
        // void setTrigger(char const *start, char const *stop);

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
        void resetBuff(void);
        bool readBuff(uint8_t &d);
        bool readBuff(void *d, uint8_t len);
        uint32_t getBufDataLen(void);
        bool checkTriggerState();
        // void addCMD(void (**f)(uint8_t *, uint32_t), uint32_t len);
        void addCMD(void (*f)(uint8_t *, uint32_t));

        WyIstream4MCU &operator>>(uint8_t &d);
    };

}

#endif /* C2342E4A_0F6A_4A86_A696_31EBC200D0B3 */
