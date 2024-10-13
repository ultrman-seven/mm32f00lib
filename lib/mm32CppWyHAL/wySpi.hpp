#ifndef E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54
#define E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54

#include "stdint.h"
#include "reg_spi.h"
#include "wyGpio.hpp"
namespace SPI
{
    class SpiObject
    {
    private:
        SPI_TypeDef *spi;

    public:
        SpiObject(uint8_t num, const char *sclk, const char *mosi, const char *miso = nullptr);
        // ~SpiObject();
        uint8_t readWrite(uint8_t);
        void readWrite(uint8_t *txBuf, uint8_t *rxBuf, uint32_t len);
    };

} // namespace SPI

#endif /* E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54 */
