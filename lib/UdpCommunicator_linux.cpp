#include "Logger.h"
#include "UdpCommunicator.h"

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

namespace Visca {
struct UdpCommunicator::Impl {
    struct sockaddr_in remoteAddr;
};

UdpCommunicator::UdpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode)
    : m_ip(ip)
    , m_port(port)
    , m_mode(mode)
    , m_pImpl(std::make_unique<Impl>())
{
}

UdpCommunicator::~UdpCommunicator() { close(); }

bool UdpCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket >= 0)
        return true;

    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_socket < 0)
        return false;

    memset(&m_pImpl->remoteAddr, 0, sizeof(m_pImpl->remoteAddr));
    m_pImpl->remoteAddr.sin_family = AF_INET;
    m_pImpl->remoteAddr.sin_port = htons(m_port);

    if (inet_pton(AF_INET, m_ip.c_str(), &m_pImpl->remoteAddr.sin_addr) <= 0) {
        VISCALOG_ERROR("UDP: Invalid address " + m_ip);
        ::close(m_socket);
        m_socket = -1;
        return false;
    }

    if (m_mode == NetworkMode::Server) {
        struct sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = INADDR_ANY;
        localAddr.sin_port = htons(m_port);
        if (bind(m_socket, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
            VISCALOG_ERROR("UDP: Failed to bind socket");
            ::close(m_socket);
            m_socket = -1;
            return false;
        }
    }
    return true;
}

bool UdpCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket < 0)
        return false;
    ssize_t sent = ::sendto(
        m_socket, data.data(), data.size(), 0, (struct sockaddr*)&m_pImpl->remoteAddr, sizeof(m_pImpl->remoteAddr));
    return sent == static_cast<ssize_t>(data.size());
}

size_t UdpCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket < 0)
        return 0;
    struct sockaddr_in src;
    socklen_t len = sizeof(src);
    ssize_t received = ::recvfrom(m_socket, buffer, maxSize, 0, (struct sockaddr*)&src, &len);
    // In Server mode, we update remoteAddr to reply to the last sender
    if (m_mode == NetworkMode::Server && received > 0) {
        m_pImpl->remoteAddr = src;
    }
    return (received > 0) ? static_cast<size_t>(received) : 0;
}

bool UdpCommunicator::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket >= 0;
}

void UdpCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket >= 0) {
        ::close(m_socket);
        m_socket = -1;
        VISCALOG_INFO("UDP socket closed.");
    }
}
}
