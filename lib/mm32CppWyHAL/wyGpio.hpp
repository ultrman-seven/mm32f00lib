#ifndef __MM32_CPP_HAL_WY_LIB_GPIO_HPP__
#define __MM32_CPP_HAL_WY_LIB_GPIO_HPP__
#include "reg_gpio.h"

namespace GPIO
{
    typedef enum
    {                                //  bit 7654 3210
        Mode_AIN = 0x00,             // = 0b 0000 0000,
        Mode_IN_FLOATING,            // = 0b 0000 0001,
        Mode_IPD,                    // = 0b 0000 0010,
        Mode_IPU = 0x12,             // = 0b 0001 0010,
        Mode_Out_OD = 0x05,          // = 0b 0000 0101,
        Mode_Out_OD_PD = 0x15,       // = 0b 0001 0101,
        Mode_Out_OD_Floating = 0x25, // = 0b 0010 0101,
        Mode_Out_OD_PU = 0x35,       // = 0b 0011 0101,
        Mode_Out_PP = 0x04,          // = 0b 0000 0100,
        Mode_AF_OD = 0x07,           // = 0b 0000 0111,
        Mode_AF_OD_PD = 0x17,        // = 0b 0001 0111,
        Mode_AF_OD_Floating = 0x27,  // = 0b 0010 0111,
        Mode_AF_OD_PU = 0x37,        // = 0b 0011 0111,
        Mode_AF_PP = 0x06,           // = 0b 0000 0110
    } Mode;
    typedef enum
    {
        Speed_10MHz = 1,
        Speed_2MHz,
        Speed_50MHz
    } Speed;

    extern "C"
    {
        typedef struct
        {
            uint8_t fsm;
            uint8_t antiTickCnt;
            uint8_t mulCnt;
            uint16_t longTickCnt;
            void (*cb)(uint8_t, void *);
            void (*mulCb)(uint8_t, void *);
            void *arg;
        } __KeyBaseHandle_t;

#define KeyPressKind_Short 0
#define KeyPressKind_Long 1
    }

    void afConfig(const char *pin, uint8_t af, Mode m);
    void modeConfig(const char *, Mode m, Speed s = Speed_50MHz);
    void modeConfig(GPIO_TypeDef *, uint8_t p, Mode m, Speed s = Speed_50MHz);
    void extiConfig(const char *, void (*)(void), Mode = Mode_IPU);
    uint8_t afTable2afVal(char const *p, uint16_t const *table);

    class GpioPin
    {
    private:
        uint16_t pin;
        // uint16_t pinNum;
        struct
        {
            uint16_t pinNum : 8;
            uint16_t portNum : 8;
        };
        GPIO_TypeDef *port;
        Mode currentMode;

    public:
        GpioPin(const char *, Mode = Mode::Mode_Out_PP, Speed = Speed_50MHz);
        GpioPin() = default;
        void copy(GpioPin &o);
        void reInit(const char *, Mode = Mode::Mode_Out_PP, Speed = Speed_50MHz);
        bool available();

        void set(void);
        void reset(void);
        // void setOnOff(bool);
        void flip();
        void setMode(Mode m, Speed s = Speed_50MHz);
        void wait(bool);
        // void setSpeed(GpioSpeed s);

        bool read(void);
        bool getOutputState(void);
        void operator=(bool s);
        GpioPin &operator=(GpioPin &o);
        bool operator!(void);
        GpioPin &operator<<(bool);
        GpioPin &operator>>(bool &);
        void setExti(void (*callback)(void) = nullptr, uint8_t priority = 1);
        bool isTriggered();
        void triggerFlagReset();
    };

    class KeyDualEdge
    {
    private:
        uint8_t pinNum;
        uint8_t crtEdge;
        GPIO_TypeDef *gpio;
        __KeyBaseHandle_t highTrig;
        __KeyBaseHandle_t lowTrig;
        // uint32_t timestamp;

    public:
        KeyDualEdge(const char *pinName);
        void setHighCbk(void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *));
        void setLowCbk(void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *));
        void loopTick();
    };

    class Key
    {
    private:
        uint8_t pinNum;
        uint8_t activeLevel;
        GPIO_TypeDef *gpio;
        __KeyBaseHandle_t base;

    public:
        Key(const char *pinName, uint8_t activeLevel, void *arg, void (*cbk)(uint8_t, void *), void (*mul)(uint8_t, void *));
        bool isActive();
        void loopTick();
    };
#define __GPIO_AF_Val(__port, __pin, __af) (((__port) << 12) + ((__pin) << 8) + (__af))
} // namespace GPIO

#endif /* __MM32_CPP_HAL_WY_LIB_GPIO_HPP__ */
