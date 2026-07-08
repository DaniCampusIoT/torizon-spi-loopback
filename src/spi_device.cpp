#include "spi_device.hpp"

#include <cstring>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
void throwSystemError(const std::string& message) {
    throw std::runtime_error(message + ": " + std::strerror(errno));
}
}

SpiDevice::SpiDevice(const Config& config)
    : config_(config) {}

SpiDevice::~SpiDevice() {
    close();
}

SpiDevice::SpiDevice(SpiDevice&& other) noexcept
    : fd_(other.fd_), config_(other.config_) {
    other.fd_ = -1;
}

SpiDevice& SpiDevice::operator=(SpiDevice&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        config_ = other.config_;
        other.fd_ = -1;
    }
    return *this;
}

void SpiDevice::open() {
    if (isOpen()) {
        return;
    }

    fd_ = ::open(config_.device.c_str(), O_RDWR);
    if (fd_ < 0) {
        throwSystemError("Cannot open SPI device " + config_.device);
    }

    configure();
}

void SpiDevice::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool SpiDevice::isOpen() const noexcept {
    return fd_ >= 0;
}

void SpiDevice::configure() {
    if (ioctl(fd_, SPI_IOC_WR_MODE, &config_.mode) == -1) {
        throwSystemError("Cannot set SPI mode");
    }

    if (ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &config_.bitsPerWord) == -1) {
        throwSystemError("Cannot set SPI bits per word");
    }

    if (ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &config_.speedHz) == -1) {
        throwSystemError("Cannot set SPI max speed");
    }
}

std::vector<std::uint8_t> SpiDevice::transfer(const std::vector<std::uint8_t>& tx) {
    std::vector<std::uint8_t> rx(tx.size(), 0);
    transfer(tx, rx);
    return rx;
}

void SpiDevice::transfer(const std::vector<std::uint8_t>& tx, std::vector<std::uint8_t>& rx) {
    if (!isOpen()) {
        throw std::runtime_error("SPI device is not open");
    }

    if (rx.size() != tx.size()) {
        rx.resize(tx.size(), 0);
    }

    spi_ioc_transfer tr{};
    tr.tx_buf = reinterpret_cast<unsigned long>(tx.data());
    tr.rx_buf = reinterpret_cast<unsigned long>(rx.data());
    tr.len = static_cast<__u32>(tx.size());
    tr.speed_hz = config_.speedHz;
    tr.bits_per_word = config_.bitsPerWord;
    tr.delay_usecs = config_.delayUsec;
    tr.cs_change = 0;

    if (ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) < 1) {
        throwSystemError("SPI transfer failed");
    }
}

const SpiDevice::Config& SpiDevice::config() const noexcept {
    return config_;
}