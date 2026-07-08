# torizon-spi-loopback

SPI loopback test in C++ for **Apalis i.MX8** running **Torizon OS**.

Verifies the SPI bus is accessible from a Docker container using the Linux `spidev` userspace interface (`ioctl()`). MOSI and MISO are physically bridged; every transmitted byte must come back unchanged.

## Hardware

| Field | Value |
|---|---|
| SoM | Toradex Apalis i.MX8 |
| Carrier | Apalis Evaluation Board |
| OS | Torizon OS |
| SPI device | `/dev/apalis-spi1-cs0` |
| Connection | MOSI ↔ MISO (loopback wire) |

## Project structure

```
torizon-spi-loopback/
├── include/
│   └── spi_device.hpp       # SpiDevice class declaration
├── src/
│   ├── spi_device.cpp       # spidev ioctl() implementation
│   └── main.cpp             # Loopback test logic
├── CMakeLists.txt
├── docker-compose.yml
├── Dockerfile
├── Dockerfile.debug
├── Dockerfile.sdk
├── torizonPackages.json
└── .gitignore
```

## How it works

1. Opens `/dev/apalis-spi1-cs0` with `O_RDWR`.
2. Configures mode, bits-per-word and max speed via `SPI_IOC_WR_*` ioctls.
3. Sends three different byte patterns using `SPI_IOC_MESSAGE(1)` (full-duplex).
4. Compares TX and RX byte-by-byte. Exits `0` on success, `1` on mismatch or error.

## Build

```bash
# Cross-compile for ARM64 (inside the Torizon VS Code extension workflow)
cmake -DCMAKE_BUILD_TYPE=Debug -B build-arm64
cmake --build build-arm64
```

## Container setup

The SPI node must be exposed to the container. See `docker-compose.yml`:

```yaml
devices:
  - "/dev/apalis-spi1-cs0:/dev/apalis-spi1-cs0"
```

## Expected output

```
TX: 0xa5 0x5a 0x00 0xff 0x12 0x34 0xbe 0xef
RX: 0xa5 0x5a 0x00 0xff 0x12 0x34 0xbe 0xef
TX: 0x00 0x00 0x00 0x00 0xff 0xff 0xff 0xff
RX: 0x00 0x00 0x00 0x00 0xff 0xff 0xff 0xff
TX: 0xaa 0x55 0xaa 0x55 0x11 0x22 0x44 0x88
RX: 0xaa 0x55 0xaa 0x55 0x11 0x22 0x44 0x88
SPI loopback OK
```

## License

MIT — see [LICENSE](LICENSE).
