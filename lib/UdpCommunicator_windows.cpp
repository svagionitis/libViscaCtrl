#include "Logger.h"
#include "UdpCommunicator.h"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace Visca {
struct UdpCommunicator::Impl {
    sockaddr_in remoteAddr;
};

UdpCommunicator::UdpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode)
    : m_ip(ip)
    , m_port(port)
    , m_mode(mode)
    , m_pImpl(std::make_unique<Impl>())
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

UdpCommunicator::~UdpCommunicator()
{
    close();
    WSACleanup();
}

bool UdpCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket != -1)
        return true;

    m_socket = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == -1)
        return false;

    memset(&m_pImpl->remoteAddr, 0, sizeof(m_pImpl->remoteAddr));
    m_pImpl->remoteAddr.sin_family = AF_INET;
    m_pImpl->remoteAddr.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &m_pImpl->remoteAddr.sin_addr);

    if (m_mode == NetworkMode::Server) {
        sockaddr_in localAddr;
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = INADDR_ANY;
        localAddr.sin_port = htons(m_port);
        if (bind(m_socket, (SOCKADDR*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
            closesocket(m_socket);
            m_socket = -1;
            return false;
        }
    }
    return true;
}

bool UdpCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket == -1)
        return false;
    return sendto(m_socket, (const char*)data.data(), (int)data.size(), 0, (sockaddr*)&m_pImpl->remoteAddr,
               sizeof(m_pImpl->remoteAddr))
        != SOCKET_ERROR;
}

size_t UdpCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket == -1)
        return 0;
    sockaddr_in from;
    int fromLen = sizeof(from);
    int res = recvfrom(m_socket, (char*)buffer, (int)maxSize, 0, (sockaddr*)&from, &fromLen);
    if (m_mode == NetworkMode::Server && res > 0) {
        m_pImpl->remoteAddr = from;
    }
    return (res > 0) ? (size_t)res : 0;
}

bool UdpCommunicator::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket != -1;
}

void UdpCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket != -1) {
        closesocket(m_socket);
        m_socket = -1;
    }
}
}
