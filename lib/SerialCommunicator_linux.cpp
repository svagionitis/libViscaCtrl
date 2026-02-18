#include "Logger.h"
#include "SerialCommunicator.h"

#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace Visca {

SerialCommunicator::SerialCommunicator(const std::string& device, uint32_t baudRate)
    : m_device(device)
    , m_baudRate(baudRate)
{
    // We don't call open() in constructor to allow the user to control timing,
    // or they can call it immediately after construction.
}

SerialCommunicator::~SerialCommunicator() { close(); }

bool SerialCommunicator::open()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd >= 0)
        return true;

    m_fd = ::open(m_device.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (m_fd < 0) {
        VISCALOG_ERROR("Failed to open serial port: " + m_device);
        return false;
    }

    struct termios tty;
    if (tcgetattr(m_fd, &tty) != 0) {
        VISCALOG_ERROR("Serial: Error from tcgetattr");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    // Set Baud Rate (Assuming 115200 for Helios)
    speed_t speed = B115200;
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(m_fd, TCSANOW, &tty) != 0) {
        VISCALOG_ERROR("Serial: Error from tcsetattr");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    VISCALOG_INFO("Serial port opened: " + m_device);
    return true;
}

bool SerialCommunicator::send(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd < 0)
        return false;
    ssize_t written = ::write(m_fd, data.data(), data.size());
    return written == static_cast<ssize_t>(data.size());
}

size_t SerialCommunicator::receive(uint8_t* buffer, size_t maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd < 0)
        return 0;
    ssize_t bytesRead = ::read(m_fd, buffer, maxSize);
    return (bytesRead > 0) ? static_cast<size_t>(bytesRead) : 0;
}

bool SerialCommunicator::isOpen() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_fd >= 0;
}

void SerialCommunicator::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
        VISCALOG_INFO("Serial port closed.");
    }
}

}
