#include "Commands.h"
#include "Logger.h"
#include "SerialCommunicator.h"
#include "TcpCommunicator.h"
#include "UdpCommunicator.h"
#include "ViscaController.h"
#include <iostream>
#include <memory>
#include <thread>

using namespace Visca;

int main(int argc, char* argv[])
{
    // Parse command line arguments
    std::string connectionType = "serial";
    std::string device = "/dev/ttyUSB0";
    int baudRate = 9600;
    std::string ip = "192.168.1.100";
    int port = 5678;

    if (argc > 1)
        connectionType = argv[1];

    if (connectionType == "serial" && argc > 2)
        device = argv[2];
    if (connectionType == "serial" && argc > 3)
        baudRate = std::stoi(argv[3]);

    if ((connectionType == "tcp" || connectionType == "udp") && argc > 2)
        ip = argv[2];
    if ((connectionType == "tcp" || connectionType == "udp") && argc > 3)
        port = std::stoi(argv[3]);

    // Set log level
    Logger::instance().setLevel(LogLevel::Info);

    std::cout << "VISCA Camera Control CLI" << std::endl;
    std::cout << "Connection type: " << connectionType << std::endl;

    // Create appropriate communicator
    std::unique_ptr<ICommunicator> communicator;

    if (connectionType == "serial") {
        std::cout << "Device: " << device << " at " << baudRate << " baud" << std::endl;
        communicator = std::make_unique<SerialCommunicator>(device, baudRate);
    } else if (connectionType == "tcp") {
        std::cout << "TCP: " << ip << ":" << port << " (Client mode)" << std::endl;
        communicator = std::make_unique<TcpCommunicator>(ip, port, NetworkMode::Client);
    } else if (connectionType == "udp") {
        std::cout << "UDP: " << ip << ":" << port << " (Client mode)" << std::endl;
        communicator = std::make_unique<UdpCommunicator>(ip, port, NetworkMode::Client);
    } else {
        std::cerr << "Unknown connection type: " << connectionType << std::endl;
        return 1;
    }

    // Create controller
    ViscaController camera(std::move(communicator));

    if (!camera.connect()) {
        std::cerr << "Failed to connect to camera" << std::endl;
        return 1;
    }

    std::cout << "Connected to camera" << std::endl;

    // Power on
    std::cout << "Powering on..." << std::endl;
    if (!camera.execute(Command::powerOn())) {
        std::cerr << "Failed to power on camera" << std::endl;
        return 1;
    }

    // Wait for camera to initialize
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Get version info
    auto version = camera.getVersionInfo();
    std::cout << "Camera version - Vendor: 0x" << std::hex << version.vendorId << " Model: 0x" << version.modelId
              << " ROM: 0x" << version.romRevision << std::dec << std::endl;

    // Zoom in
    std::cout << "Zooming in..." << std::endl;
    camera.execute(Command::zoomTeleVariable(1, 3));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    camera.execute(Command::zoomStop());

    // Get zoom position
    uint16_t zoomPos = camera.getZoomPosition();
    std::cout << "Zoom position: " << zoomPos << std::endl;

    // Set auto focus
    std::cout << "Setting auto focus..." << std::endl;
    camera.execute(Command::focusAuto());

    // Zoom out
    std::cout << "Zooming out..." << std::endl;
    camera.execute(Command::zoomWideVariable(1, 3));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    camera.execute(Command::zoomStop());

    std::cout << "Done. Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
