#include "Logger.h"
#include "TcpCommunicator.h"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace Visca {
TcpCommunicator::TcpCommunicator(const std::string& ip, uint16_t port, NetworkMode mode)
    : m_ip(ip)
    , m_port(port)
    , m_mode(mode)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

TcpCommunicator::~TcpCommunicator()
{
    close();
    WSACleanup();
}

bool TcpCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket != -1)
        return true;

    DWORD timeout = 1000; // 1 second timeout

    if (m_mode == NetworkMode::Client) {
        m_socket = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

        sockaddr_in clientService;
        clientService.sin_family = AF_INET;
        inet_pton(AF_INET, m_ip.c_str(), &clientService.sin_addr);
        clientService.sin_port = htons(m_port);

        if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
            closesocket(m_socket);
            m_socket = -1;
            return false;
        }
    } else // Server Mode
    {
        if (m_serverFd == -1) {
            m_serverFd = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            setsockopt(m_serverFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

            sockaddr_in service;
            service.sin_family = AF_INET;
            service.sin_addr.s_addr = INADDR_ANY;
            service.sin_port = htons(m_port);

            if (bind(m_serverFd, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
                closesocket(m_serverFd);
                m_serverFd = -1;
                return false;
            }
            listen(m_serverFd, 1);
        }

        m_socket = (int)accept(m_serverFd, NULL, NULL);
        if (m_socket != -1) {
            setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        }
    }
    return m_socket != -1;
}

bool TcpCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket == -1)
        return false;
    return ::send(m_socket, (const char*)data.data(), (int)data.size(), 0) != SOCKET_ERROR;
}

size_t TcpCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket == -1)
        return 0;

    int res = ::recv(m_socket, (char*)buffer, (int)maxSize, 0);

    if (res == 0) // Graceful peer disconnect
    {
        closesocket(m_socket);
        m_socket = -1;
        return 0;
    }
    return (res > 0) ? (size_t)res : 0;
}

bool TcpCommunicator::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket != -1;
}

void TcpCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_socket != -1) {
        closesocket(m_socket);
        m_socket = -1;
    }
    if (m_serverFd != -1) {
        closesocket(m_serverFd);
        m_serverFd = -1;
    }
}
}
