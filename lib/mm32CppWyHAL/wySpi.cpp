#include "wySpi.hpp"

#include "stdint.h"
namespace SPI
{
    class SpiObject
    {
    private:
    public:
        SpiObject(uint8_t num, const char *sclk, const char *mosi, const char *miso=nullptr);
        // ~SpiObject();

    };

} // namespace SPI

using namespace SPI;

SpiObject::SpiObject(uint8_t num, const char *sclk, const char *mosi, const char *miso)
{
}
