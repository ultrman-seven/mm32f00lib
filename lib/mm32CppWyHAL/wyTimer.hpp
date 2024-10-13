#ifndef B1B7122D_239F_405E_976E_DDD2E44039B7
#define B1B7122D_239F_405E_976E_DDD2E44039B7

#include "stdint.h"
#include "reg_tim.h"

namespace TIM
{
    typedef uint32_t *pwmType;
    pwmType pwm(uint8_t timx, uint8_t chx, const char *p, const char *n, uint32_t period, uint32_t psc);

    typedef enum
    {
        Mode_Up,
        Mode_Down,
        Mode_CenterAligned1,
        Mode_CenterAligned2,
        Mode_CenterAligned3
    } Mode;
    class Timer
    {
    private:
        TIM_TypeDef *tim;
        uint8_t numIdx;

    public:
        Timer(uint8_t timx, uint32_t period, uint32_t psc, uint8_t div, Mode m = Mode_Up);
        Timer(uint8_t timx, uint32_t usPeriod);
        // ~Timer();

        bool updated();
        pwmType setPwm(uint8_t ch, const char *p, const char *n = nullptr);
        void interruptConfig(void (*callback)(void), uint8_t p = 1);
        void interruptCMD(bool s);
        void work(bool s);
        void clear(void) { tim->CNT = 0; }
        uint32_t getCnt(void) { return tim->CNT; }
    };
} // namespace TIM

#endif /* B1B7122D_239F_405E_976E_DDD2E44039B7 */
