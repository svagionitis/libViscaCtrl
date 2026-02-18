#include "Logger.h"
#include "TcpCommunicator.h"

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace Visca {
TcpCommunicator::TcpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode)
    : m_ip(ip)
    , m_port(port)
    , m_mode(mode)
{
}

TcpCommunicator::~TcpCommunicator() { close(); }

bool TcpCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket >= 0)
        return true;

    struct timeval tv;
    tv.tv_sec = 1; // 1-second timeout for responsive shutdown
    tv.tv_usec = 0;

    if (m_mode == NetworkMode::Client) {
        m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
            return false;

        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_port);
        inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr);

        if (::connect(m_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            ::close(m_socket);
            m_socket = -1;
            return false;
        }
    } else // Server Mode
    {
        if (m_serverFd < 0) {
            m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
            int opt = 1;
            setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(m_serverFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(m_port);

            if (::bind(m_serverFd, (struct sockaddr*)&addr, sizeof(addr)) < 0 || listen(m_serverFd, 1) < 0) {
                if (m_serverFd >= 0)
                    ::close(m_serverFd);
                m_serverFd = -1;
                return false;
            }
        }

        m_socket = accept(m_serverFd, nullptr, nullptr);
        if (m_socket >= 0) {
            setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        }
    }
    return m_socket >= 0;
}

bool TcpCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket < 0)
        return false;
    ssize_t sent = ::send(m_socket, data.data(), data.size(), 0);
    return sent == static_cast<ssize_t>(data.size());
}

size_t TcpCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket < 0)
        return 0;

    ssize_t received = ::recv(m_socket, buffer, maxSize, 0);

    if (received == 0) // Peer disconnected
    {
        ::close(m_socket);
        m_socket = -1;
        return 0;
    }
    return (received > 0) ? static_cast<size_t>(received) : 0;
}

bool TcpCommunicator::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket >= 0;
}

void TcpCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket >= 0) {
        ::close(m_socket);
        m_socket = -1;
    }
    if (m_serverFd >= 0) {
        ::close(m_serverFd);
        m_serverFd = -1;
    }
}
}
