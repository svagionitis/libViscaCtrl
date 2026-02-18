#include "Logger.h"
#include "SerialCommunicator.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Visca {

struct SerialInternal {
    HANDLE handle = INVALID_HANDLE_VALUE;
};

SerialCommunicator::SerialCommunicator(const std::string& device, uint32_t baudRate)
    : m_device("\\\\.\\" + device)
    , m_baudRate(baudRate)
    , m_fd(-1)
{
    // Note: We use m_fd as a dummy for the interface, actual logic uses HANDLE
}

SerialCommunicator::~SerialCommunicator() { close(); }

bool SerialCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd != -1)
        return true;

    HANDLE hSerial = CreateFileA(m_device.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        VISCALOG_ERROR("WinSerial: Failed to open " + m_device);
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return false;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    m_fd = static_cast<int>(reinterpret_cast<intptr_t>(hSerial));
    VISCALOG_INFO("WinSerial: Opened " + m_device);
    return true;
}

bool SerialCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd == -1)
        return false;
    DWORD bytesWritten;
    return WriteFile(
        reinterpret_cast<HANDLE>(static_cast<intptr_t>(m_fd)), data.data(), (DWORD)data.size(), &bytesWritten, NULL);
}

size_t SerialCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd == -1)
        return 0;
    DWORD bytesRead;
    if (ReadFile(reinterpret_cast<HANDLE>(static_cast<intptr_t>(m_fd)), buffer, (DWORD)maxSize, &bytesRead, NULL)) {
        return (size_t)bytesRead;
    }
    return 0;
}

bool SerialCommunicator::isOpen() const { return m_fd != -1; }

void SerialCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd != -1) {
        CloseHandle(reinterpret_cast<HANDLE>(static_cast<intptr_t>(m_fd)));
        m_fd = -1;
    }
}

}
