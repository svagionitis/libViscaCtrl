# ViscaCtrl - VISCA Camera Control Library

A cross-platform C++17 library for controlling Sony VISCA protocol cameras (FCB series) over serial, TCP, and UDP connections. Designed with RAII principles and suitability for embedded devices in mind.

## Features

- **Full VISCA protocol support** for Sony FCB camera series (EV9500L, etc.)
- **Multiple communication interfaces**:
  - Serial (RS-232C) with configurable baud rates
  - TCP client/server modes
  - UDP client/server modes
- **Thread-safe** design with mutex protection
- **Ring buffer** for efficient data handling
- **Thread-safe logging** system
- **RAII compliant** - automatic resource management
- **CMake based** build system
- **No external dependencies** - pure C++17
- **Cross-platform** - Windows and Linux support
- **Doxygen-style comments** for API documentation
- **WebKit code style** with .clang-format configuration

## Project Structure

```
viscactrl/
├── .clang-format              # WebKit code style configuration
├── .editorconfig              # Editor configuration
├── .gitignore                 # Comprehensive git ignore rules
├── CMakeLists.txt             # Root CMake configuration
├── LICENSE                    # MIT License
├── cmake/
│   └── AddGTest.cmake         # GoogleTest integration
├── lib/                       # Core library
│   ├── CMakeLists.txt
│   ├── Commands.h             # VISCA command definitions
│   ├── Commands.cpp
│   ├── Export.h                # DLL export/import macros
│   ├── ICommunicator.h         # Communication interface
│   ├── Logger.h                # Thread-safe logging
│   ├── Logger.cpp
│   ├── RingBuffer.h            # Thread-safe ring buffer
│   ├── SerialCommunicator.h
│   ├── SerialCommunicator_linux.cpp
│   ├── SerialCommunicator_windows.cpp
│   ├── TcpCommunicator.h
│   ├── TcpCommunicator_linux.cpp
│   ├── TcpCommunicator_windows.cpp
│   ├── UdpCommunicator.h
│   ├── UdpCommunicator_linux.cpp
│   ├── UdpCommunicator_windows.cpp
│   ├── UtilsCommon.h           # Utility functions
│   ├── ViscaController.h
│   └── ViscaController.cpp
├── ClViscaCli/                 # Command-line client
│   ├── CMakeLists.txt
│   └── main.cpp
├── QtViscaCli/                 # Qt GUI client (optional)
│   └── CMakeLists.txt
├── tests/                      # Unit tests (optional)
└── docs/                       # Documentation
    ├── DEEPSEEK-Prompt.md      # Original architecture prompt
    ├── FCB-EV9500L_TM_EN_20220117.pdf  # Sony camera manual
    └── FCB_control_Software_v5-2-0.zip  # Sony control software
```

## Building

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler (GCC, Clang, MSVC)
- For Windows: Visual Studio 2019 or later

### Build Instructions

```bash
git clone <repository-url>
cd viscactrl
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DENABLE_CL_CLI=ON \        # Build command-line client
         -DENABLE_QT_CLI=OFF \        # Build Qt client (requires Qt)
         -DBUILD_TESTS=OFF \          # Build unit tests
         -DBUILD_SHARED_LIBS=OFF      # Build static library

# Build
cmake --build . -- -j$(nproc)        # Linux
cmake --build . -- /maxcpucount       # Windows (Visual Studio)
```

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `ENABLE_CL_CLI` | Build command-line client | ON |
| `ENABLE_QT_CLI` | Build Qt GUI client | OFF |
| `BUILD_TESTS` | Build unit tests | OFF |
| `BUILD_SHARED_LIBS` | Build shared library | OFF |
| `NON_TRANSITIVE` | Use non-transitive linking | OFF |

## Usage

### Command-Line Client

The command-line client demonstrates basic camera control:

```bash
# Serial connection (default)
./ClViscaCli serial /dev/ttyUSB0 9600

# TCP connection
./ClViscaCli tcp 192.168.1.100 5678

# UDP connection
./ClViscaCli udp 192.168.1.100 5678
```

### Library Usage Example

```cpp
#include <ViscaController.h>
#include <SerialCommunicator.h>
#include <memory>

using namespace Visca;

int main() {
    // Create serial communicator
    auto comm = std::make_unique<SerialCommunicator>("/dev/ttyUSB0", 9600);

    // Create controller
    ViscaController camera(std::move(comm));

    // Connect to camera
    if (!camera.connect()) {
        return 1;
    }

    // Power on
    camera.execute(Command::powerOn());

    // Zoom in
    camera.execute(Command::zoomTeleVariable(1, 3));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    camera.execute(Command::zoomStop());

    // Get zoom position
    uint16_t zoomPos = camera.getZoomPosition();
    std::cout << "Zoom position: " << zoomPos << std::endl;

    // Get version info
    auto version = camera.getVersionInfo();
    std::cout << "Vendor: 0x" << std::hex << version.vendorId
              << " Model: 0x" << version.modelId << std::endl;

    return 0;
}
```

## Key Components

### Commands
The `Commands.h` provides factory methods for all VISCA commands:
- Power control
- Zoom control (standard/variable/direct)
- Focus control (auto/manual/one-push)
- White balance modes
- Exposure modes
- Inquiries (version, position, status)

### Communicators
Three communication implementations:
- **SerialCommunicator**: RS-232C communication
- **TcpCommunicator**: TCP client/server
- **UdpCommunicator**: UDP client/server

All implement the `ICommunicator` interface for easy swapping.

### ViscaController
Main controller class that:
- Manages the communication thread
- Handles command/response flow (acknowledge/completion)
- Provides both synchronous and asynchronous APIs
- Maintains thread-safe receive buffer

### Logger
Thread-safe logging with levels:
- Error
- Warning
- Info (default)
- Debug

```cpp
Logger::instance().setLevel(LogLevel::Debug);
VISCALOG_INFO("Camera connected");
VISCALOG_DEBUG("Sending command: " << cmd.packet());
```

### RingBuffer
Thread-safe circular buffer template for efficient data handling between threads.

## VISCA Protocol Support

The library implements the Sony VISCA protocol as documented in the FCB-EV9500L technical manual:

- **Addressing**: Support for multiple camera addresses (1-7)
- **Command structure**: Proper header (8x), category, command, parameters, terminator (FF)
- **Response handling**: Acknowledge (40-4F), Completion (50-5F), Error (60-6F)
- **Socket numbers**: Support for two command buffers
- **Inquiry commands**: Full support for all inquiries
- **Broadcast commands**: Support for IF_Clear, AddressSet

## Acknowledgments

- Sony Corporation for the VISCA protocol documentation
- Based on the FCB-EV9500L camera module technical manual
