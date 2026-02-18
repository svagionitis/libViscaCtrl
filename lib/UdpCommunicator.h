#pragma once

#include "ICommunicator.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Visca {
/**
 * @brief Cross-platform UDP Communicator using the Pimpl pattern.
 */
class VISCA_EXPORT UdpCommunicator : public ICommunicator {
public:
    UdpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode);
    ~UdpCommunicator() override;

    bool open() override;
    bool send(const std::vector<uint8_t>& data) override;
    size_t receive(uint8_t* buffer, size_t maxSize) override;
    bool isOpen() const override;
    void close() override;

private:
    int m_socket { -1 };
    std::string m_ip;
    uint16_t m_port;
    NetworkMode m_mode;
    mutable std::mutex m_mutex;

    // Forward declaration of the platform-specific implementation
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};
}
