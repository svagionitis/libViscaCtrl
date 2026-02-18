#pragma once

#include "ICommunicator.h"
#include <cstdint>
#include <mutex>
#include <string>

namespace Visca {

/**
 * @brief Linux implementation of Serial communication (RS232/RS422).
 */
class VISCA_EXPORT SerialCommunicator : public ICommunicator {
public:
    SerialCommunicator(const std::string& device, uint32_t baudRate);
    ~SerialCommunicator() override;

    bool open() override;
    bool send(const std::vector<uint8_t>& data) override;
    size_t receive(uint8_t* buffer, size_t maxSize) override;
    bool isOpen() const override;
    void close() override;

private:
    int m_fd { -1 };
    std::string m_device;
    uint32_t m_baudRate;
    mutable std::mutex m_mutex;
};
}
