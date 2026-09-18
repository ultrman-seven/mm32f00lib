#ifndef E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54
#define E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54

#include "stdint.h"
#include "reg_spi.h"
#include "wyGpio.hpp"

#define __Spi_ClkCfg_PhaseEdge1 0x0001
#define __Spi_ClkCfg_PhaseEdge2 0x0000
#define __Spi_ClkCfg_PolarityIdleHigh 0x0002
#define __Spi_ClkCfg_PolarityIdleLow 0x0000

#define Spi_ClkCfg_Edge1High (__Spi_ClkCfg_PhaseEdge1 | __Spi_ClkCfg_PolarityIdleHigh)
#define Spi_ClkCfg_Edge2High (__Spi_ClkCfg_PhaseEdge2 | __Spi_ClkCfg_PolarityIdleHigh)
#define Spi_ClkCfg_Edge1Low (__Spi_ClkCfg_PhaseEdge1 | __Spi_ClkCfg_PolarityIdleLow)
#define Spi_ClkCfg_Edge2Low (__Spi_ClkCfg_PhaseEdge2 | __Spi_ClkCfg_PolarityIdleLow)

namespace SPI
{
    class SpiObject
    {
    private:
        SPI_TypeDef *spi;
        void (*cbk)(void *);
        void *arg;
        uint8_t *txBuf;
        uint8_t *rxBuf;
        uint32_t txLen;
        uint32_t rxLen;
        friend void spiIrqHandle(SpiObject *handle);

    public:
        SpiObject(uint8_t num, const char *sclk,
                  const char *mosi = nullptr, const char *miso = nullptr,
                  GPIO::Mode mosiMode = GPIO::Mode_AF_PP,
                  uint16_t clkDiv = 8, uint8_t clockCfg = Spi_ClkCfg_Edge1High);
        // ~SpiObject();
        uint8_t readWrite(uint8_t);
        void readWriteAsync(uint8_t *txBuf, uint8_t *rxBuf, uint32_t len, void (*cbk)(void *), void *arg);
        void readAsync(uint8_t *buf, uint32_t len, void (*cbk)(void *), void *arg);
        void writeAsync(uint8_t *buf, uint32_t len, void (*cbk)(void *), void *arg);
    };

} // namespace SPI

#endif /* E0DC5743_3B5A_4514_8D6E_EB8B7C9C2F54 */
