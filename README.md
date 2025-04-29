# Apollo-RTOS

> An AGC-Inspired Real-Time Operating System for Embedded Systems

```
   _____         _ _        _____ _____ _____ _____
  |  _  |___ ___| | |___   | __  |_   _|     |   __|
  |     | . | . | | | . |  |    -| | | |  |  |__   |
  |__|__|  _|___|_|_|___|  |__|__| |_| |_____|_____|
        |_|

  Welcome to Apollo RTOS!

  boot level: boot
  stats: n_flash = 124, n_boot = 7, n_power = 6, n_reset = 99

  >> help
  pkill   htop    help    prof    accel   show
  calc    clear   echo    cat     ls      touch
  rm      heart

  >> 
```

## Overview

**Apollo-RTOS** is a real-time operating system designed for the BBC **micro:bit v1**.
Inspired by the **Apollo Guidance Computer (AGC)** — the computer that powered the
Apollo 11 moon landing — this project reimagines its resilient, restart-based
architecture in a modern embedded context.

Despite operating in a drastically more powerful environment than the AGC (2 MHz, 4 KB
RAM), embedded systems today still face core challenges: **real-time responsiveness**,
**fault tolerance**, **resource efficiency**, and **system recovery**. Apollo-RTOS
revisits historical design principles and merges them with modern C++20 features and
Unix philosophies to build a robust, modular, and highly maintainable operating system.

Key features include:

- **Hybrid cooperative-preemptive scheduler** supporting up to 16 concurrent processes
- **Restart-based fault recovery** ensuring resilience against resets and power failures
- **Persistent file systems** unifying Flash and FRAM memory
- **Unix-like shell interface** for interacting with processes, files, and system status
- **Dynamic memory management** and modular drivers (UART, I2C, Display)
- **Automated testing framework** for system verification
- **C++20** clean, type-safe, template-driven implementation

Apollo-RTOS demonstrates that principles developed for the Apollo missions still hold
value today, offering a blueprint for modern embedded system reliability under tight
resource constraints.


## Motivation

- **Historical Legacy:** The AGC demonstrated revolutionary reliability strategies under severe hardware limitations. Learning from its design teaches timeless engineering principles.
- **Modern Application:** Embedded devices, though more powerful today, still require extreme reliability with minimal resources, especially in safety-critical and mission-critical systems.
- **Educational:** Building Apollo-RTOS from scratch offers a practical exercise in real-time system design, low-level embedded programming, and C++ systems engineering.


## Target Device

**BBC micro:bit v1**
- ARM Cortex-M0 CPU
- 16 KB RAM, 256 KB Flash
- 5x5 LED display
- Serial over USB
- I2C bus for external FRAM support (optional)


## Requirements

### Hardware
- **BBC micro:bit v1**
- (Optional) **I2C FRAM chip** (e.g., Fujitsu MB85RC256V) for persistent storage testing
- **USB cable** to flash firmware onto the micro:bit

### Software
- **Toolchain:**  
  - `arm-none-eabi-gcc` (for cross-compiling)
  - `arm-none-eabi-g++`
  - `arm-none-eabi-objcopy`
  - `arm-none-eabi-objdump`
- **CMake** (>= 2.8)
- **Python 3** (for post-build scripts like stack usage analysis)
- **Minicom** (for UART communication over USB)
- (Optional) **QEMU** (for emulation/testing without hardware)

Install `gcc-arm-none-eabi`, `cmake`, `python3`, and `minicom` via your system package manager.


## Building the Project

1. Clone the repository:

```bash
git clone https://github.com/AnglyPascal/apollo-rtos.git
cd apollo-rtos
```

2. Create a build directory:

```bash
mkdir build && cd build
```

3. Configure CMake:

```bash
cmake ..
```

4. Build the project:

```bash
make apollo
```

This will generate the `apollo.hex` file ready to flash onto your micro:bit.


## Flashing to the micro:bit

1. Connect your micro:bit via USB.
2. Copy the generated HEX file:

```bash
cp apollo.hex /media/<your-username>/MICROBIT/
```

Alternatively, enable automatic copying by setting the `COPY_HEX` CMake option:

```bash
cmake -DCOPY_HEX=ON ..
make apollo
```


## Running and Interacting

- After flashing, open a serial connection:

```bash
minicom -D /dev/ttyACM0 -b 9600
```

- You will be greeted with the Apollo-RTOS shell:

```
 _____         _ _        _____ _____ _____ _____
|  _  |___ ___| | |___   | __  |_   _|     |   __|
|     | . | . | | | . |  |    -| | | |  |  |__   |
|__|__|  _|___|_|_|___|  |__|__| |_| |_____|_____|
    |_|

Welcome to Apollo RTOS!

boot level: boot
stats: n_flash = 23, n_boot = 5, n_power = 3, n_reset = 33

>>
```

You can then run built-in commands, manage processes, inspect system stats, and interact with files.


## Emulating (Optional)

If you prefer not to use hardware:

- Run Apollo-RTOS inside QEMU:

```bash
make qemu
```

- For debugging with GDB:

```bash
make qemu-gdb
```


## Project Structure

- `src/` - Core OS code: scheduler, memory management, drivers, file systems, shell
- `include/` - Header files
- `tests/` - Unit tests and system tests
- `scripts/` - Helper scripts (e.g., flashing, stack usage analysis)


## License

This project is open for educational and experimental purposes. For more information, please refer to the repository's LICENSE file.
