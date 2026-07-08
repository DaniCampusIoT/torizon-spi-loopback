#include "spi_device.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
void printBuffer(const std::string& label, const std::vector<std::uint8_t>& data) {
    std::cout << label;
    for (const auto byte : data) {
        std::cout << " 0x"
                  << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(byte);
    }
    std::cout << std::dec << '\n';
}

bool runLoopbackTest(SpiDevice& spi, const std::vector<std::uint8_t>& tx) {
    const auto rx = spi.transfer(tx);

    printBuffer("TX:", tx);
    printBuffer("RX:", rx);

    return tx == rx;
}
}

int main() {
    try {
        SpiDevice::Config config;
        config.device = "/dev/apalis-spi1-cs0";
        config.mode = 0;
        config.bitsPerWord = 8;
        config.speedHz = 500000;
        config.delayUsec = 0;

        SpiDevice spi(config);
        spi.open();

        const std::vector<std::uint8_t> pattern1{0xA5, 0x5A, 0x00, 0xFF, 0x12, 0x34, 0xBE, 0xEF};
        const std::vector<std::uint8_t> pattern2{0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
        const std::vector<std::uint8_t> pattern3{0xAA, 0x55, 0xAA, 0x55, 0x11, 0x22, 0x44, 0x88};

        bool ok = true;
        ok &= runLoopbackTest(spi, pattern1);
        ok &= runLoopbackTest(spi, pattern2);
        ok &= runLoopbackTest(spi, pattern3);

        if (ok) {
            std::cout << "SPI loopback OK\n";
            return 0;
        }

        std::cout << "SPI loopback FAILED\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}