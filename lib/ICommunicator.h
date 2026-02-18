#pragma once

#include "Export.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Visca {
/**
 * @brief Defines the role of the network communicator.
 */
enum class NetworkMode {
    Client, ///< Actively connects to a remote IP/Port
    Server ///< Listens for incoming connections on a local Port
};

/**
 * @brief Interface for communication hardware/protocols.
 */
class VISCA_EXPORT ICommunicator {
public:
    virtual ~ICommunicator() = default;

    /**
     * @brief Opens the communication channel.
     * @return true if successful.
     */
    virtual bool open() = 0;

    /**
     * @brief Sends raw data.
     * @return true if successful.
     */
    virtual bool send(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Receives raw data into a buffer.
     * @param buffer Pointer to the destination buffer.
     * @param maxSize Maximum bytes to read.
     * @return Number of bytes actually read.
     */
    virtual size_t receive(uint8_t* buffer, size_t maxSize) = 0;

    /**
     * @brief Checks if the communication channel is open/connected.
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Closes the communication channel.
     */
    virtual void close() = 0;
};
}
