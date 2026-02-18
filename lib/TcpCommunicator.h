#pragma once

#include "ICommunicator.h"
#include <cstdint>
#include <mutex>
#include <string>

namespace Visca {

class VISCA_EXPORT TcpCommunicator : public ICommunicator {
public:
    /**
     * @param ip Remote IP for Client, Local IP (e.g., "0.0.0.0") for Server.
     * @param port The target or listening port.
     * @param mode Client or Server.
     */
    TcpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode);
    ~TcpCommunicator() override;

    bool open() override;
    bool send(const std::vector<uint8_t>& data) override;
    size_t receive(uint8_t* buffer, size_t maxSize) override;
    bool isOpen() const override;
    void close() override;

private:
    int m_socket { -1 };
    int m_serverFd { -1 };
    std::string m_ip;
    uint16_t m_port;
    NetworkMode m_mode;
    mutable std::mutex m_mutex;
};
}
