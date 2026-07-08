#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SpiDevice {
public:
    struct Config {
        std::string device{"/dev/apalis-spi1-cs0"};
        std::uint8_t mode{0};
        std::uint8_t bitsPerWord{8};
        std::uint32_t speedHz{500000};
        std::uint16_t delayUsec{0};
    };

    explicit SpiDevice(const Config& config);
    ~SpiDevice();

    SpiDevice(const SpiDevice&) = delete;
    SpiDevice& operator=(const SpiDevice&) = delete;

    SpiDevice(SpiDevice&& other) noexcept;
    SpiDevice& operator=(SpiDevice&& other) noexcept;

    void open();
    void close();

    bool isOpen() const noexcept;

    std::vector<std::uint8_t> transfer(const std::vector<std::uint8_t>& tx);
    void transfer(const std::vector<std::uint8_t>& tx, std::vector<std::uint8_t>& rx);

    const Config& config() const noexcept;

private:
    void configure();

    int fd_{-1};
    Config config_;
};