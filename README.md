# torizon-spi-loopback

SPI loopback test in C++ for **Apalis i.MX8** running **Torizon OS**.

Verifies that the SPI bus is accessible from a Docker container using the Linux `spidev` userspace interface (`ioctl()`). MOSI and MISO are physically bridged; every transmitted byte must come back unchanged.

---

## Table of contents

1. [Prerequisites](#prerequisites)
2. [Hardware setup](#hardware-setup)
3. [Project structure](#project-structure)
4. [How it works](#how-it-works)
5. [Step-by-step setup](#step-by-step-setup)
6. [Build](#build)
7. [Container setup](#container-setup)
8. [Running the application](#running-the-application)
9. [Expected output](#expected-output)
10. [Troubleshooting](#troubleshooting)
11. [License](#license)

---

## Prerequisites

Before cloning this repo, make sure you have the following installed and configured:

| Tool | Version | Purpose |
|---|---|---|
| [VS Code](https://code.visualstudio.com/) | Latest | IDE |
| [Torizon IDE Extension](https://marketplace.visualstudio.com/items?itemName=toradex.torizon) | Latest | Cross-compile & deploy to board |
| [Docker Desktop](https://www.docker.com/products/docker-desktop/) | Latest | Build container images |
| [WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install) | Ubuntu recommended | Linux environment on Windows |
| Git | Any | Version control |
| Toradex board flashed with **Torizon OS** | 6.x or later | Target hardware |

You also need a **Toradex account** to push images to the Toradex container registry (`${DOCKER_LOGIN}`).

---

## Hardware setup

| Field | Value |
|---|---|
| SoM | Toradex Apalis i.MX8 |
| Carrier | Apalis Evaluation Board |
| OS | Torizon OS |
| SPI device | `/dev/apalis-spi1-cs0` |
| Connection | MOSI ↔ MISO (loopback wire) |

**Physical wiring for loopback:**  
Connect the **MOSI** pin to the **MISO** pin on the Apalis Evaluation Board expansion connector with a short jumper wire. No slave device is needed — the master reads back its own output.

> **Tip:** Check the [Apalis Evaluation Board datasheet](https://www.toradex.com/resources/product-resources?q=apalis+evaluation+board) for the exact pin numbers of MOSI and MISO on the X1/X2 expansion connectors.

---

## Project structure

```
torizon-spi-loopback/
├── include/
│   └── spi_device.hpp       # SpiDevice class declaration
├── src/
│   ├── spi_device.cpp       # spidev ioctl() implementation
│   └── main.cpp             # Loopback test logic
├── CMakeLists.txt           # Build configuration
├── docker-compose.yml       # Container definition (debug + release)
├── Dockerfile               # Release image
├── Dockerfile.debug         # Debug image (SSH + GDB server)
├── Dockerfile.sdk           # SDK image for cross-compilation
├── torizonPackages.json     # Container runtime dependencies
└── .gitignore
```

---

## How it works

1. Opens `/dev/apalis-spi1-cs0` with `O_RDWR`.
2. Configures mode, bits-per-word and max speed via `SPI_IOC_WR_*` ioctls.
3. Sends three different byte patterns using `SPI_IOC_MESSAGE(1)` (full-duplex transfer).
4. Compares TX and RX byte-by-byte. Exits `0` on success, `1` on mismatch or error.

---

## Step-by-step setup

### 1. Clone the repository

```bash
git clone https://github.com/DaniCampusIoT/torizon-spi-loopback.git
cd torizon-spi-loopback
```

### 2. Open in VS Code

```bash
code .
```

The Torizon extension will detect the project automatically. If prompted, allow it to initialize the workspace.

### 3. Configure the `.env` file

The Torizon extension generates a `.env` file with your credentials and board IP. Make sure it contains at least:

```env
DOCKER_LOGIN=your_dockerhub_or_toradex_username
IMAGE_ARCH=arm64
DEBUG_SSH_PORT=2231
TAG=latest
```

> **Note:** `.env` is listed in `.gitignore` — never commit it, it contains your credentials.

### 4. Verify the SPI device on the board

SSH into your board and confirm the SPI node exists:

```bash
ssh torizon@<board-ip>
ls /dev/apalis-spi*
# Expected: /dev/apalis-spi1-cs0  /dev/apalis-spi2-cs0
```

If the node is missing, the SPI overlay may not be enabled. See [Troubleshooting](#troubleshooting).

### 5. Connect the loopback wire

Bridge MOSI and MISO on the expansion connector before running the application.

### 6. Build and deploy with VS Code

Use the Torizon extension tasks:
- **Build Debug** → cross-compiles for ARM64 and builds the debug Docker image.
- **Deploy Debug** → pushes the image to the board and starts the container.
- **Attach Debugger** → connects GDB to the running process.

Or build manually (see [Build](#build)).

---

## Build

```bash
# Inside WSL or the board itself
cmake -DCMAKE_BUILD_TYPE=Debug -B build-arm64
cmake --build build-arm64
```

The binary is placed at `build-arm64/bin/MemoryAccess`.

---

## Container setup

The SPI device node must be explicitly exposed to the container. This is already configured in `docker-compose.yml`:

```yaml
devices:
  - "/dev/apalis-spi1-cs0:/dev/apalis-spi1-cs0"
device_cgroup_rules:
  - c 253:* rmw
  - c 199:* rmw
```

> Without the `devices` entry, `open()` on the SPI node will fail with **Permission denied** or **No such file or directory** even if the node exists on the host.

---

## Running the application

Once the container is running on the board:

```bash
# Inside the container (via VS Code terminal or SSH)
./bin/MemoryAccess
```

---

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

If TX and RX match on all three patterns → the SPI bus, the container mapping and the loopback wiring are all correct.

---

## Troubleshooting

### `/dev/apalis-spi1-cs0: No such file or directory`

The SPI node does not exist on the host.

- Verify the SPI device tree overlay is enabled in Torizon:
  ```bash
  ls /dev/apalis-spi* /dev/spidev*
  ```
- If nothing appears, the SPI interface may need to be activated via a device tree overlay. Refer to the [Toradex SPI documentation](https://developer.toradex.com/software/linux-resources/linux-features/spi-linux/) for your board.
- Make sure you are using the correct bus (`spi1` vs `spi2`). Check which one maps to the expansion connector pins you wired.

---

### `open() failed: Permission denied`

The container cannot access the device.

- Confirm the `devices` entry in `docker-compose.yml` maps the correct node:
  ```yaml
  devices:
    - "/dev/apalis-spi1-cs0:/dev/apalis-spi1-cs0"
  ```
- Re-deploy the container after editing `docker-compose.yml` — a running container does not pick up changes until it is recreated.
- Check that the file on the host is readable:
  ```bash
  ls -la /dev/apalis-spi1-cs0
  # Should show crw-rw---- or similar
  ```

---

### `SPI loopback FAILED` (TX ≠ RX)

The bytes sent do not match the bytes received.

- **No loopback wire:** The most common cause. Check that MOSI and MISO are physically bridged.
- **Wrong pins:** Double-check the connector pinout in the [Apalis Evaluation Board datasheet](https://www.toradex.com/resources/product-resources).
- **Noise or bad contact:** Try a shorter jumper wire or a proper dupont cable.
- **Speed too high:** Reduce `speedHz` in `main.cpp` from `500000` to `100000` and retest.

---

### `ioctl: Invalid argument` when setting mode or speed

- The SPI driver on your board may not support the requested mode or speed. Try `mode = 0` and `speedHz = 100000` first.
- Some Torizon image versions have slightly different spidev driver builds. Update to the latest Torizon OS if the issue persists.

---

### Debug SSH port conflict (`Bind for 0.0.0.0:2231 failed: port is already allocated`)

Another container is already using that port.

```bash
# Check which container is using the port
docker ps --format '{{.Names}}\t{{.Ports}}'

# Stop the conflicting container
docker stop <container-name>
```

Or change `DEBUG_SSH_PORT` in your `.env` file to a free port (e.g., `2232`) and redeploy.

---

### Git: `detected dubious ownership` when running from Windows PowerShell

Git blocks UNC paths (`\\wsl.localhost\...`) for security reasons.

**Fix:** Run all git commands from inside WSL, not from PowerShell:

```bash
# Open a WSL terminal and navigate to the project
cd /home/<your-user>/memoryAccess
git ...
```

Alternatively, add an exception (less recommended):

```powershell
git config --global --add safe.directory '%(prefix)///wsl.localhost/Torizon/home/<user>/memoryAccess'
```

---

### Git checkout blocked by existing local file

```
error: The following untracked working tree files would be overwritten by checkout: .gitignore
```

Remove the local copy and retry:

```bash
rm .gitignore
git checkout -b main --track origin/main
```

---

## License

MIT — see [LICENSE](LICENSE).
