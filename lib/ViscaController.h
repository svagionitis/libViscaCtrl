#pragma once

#include "Commands.h"
#include "Export.h"
#include "ICommunicator.h"
#include "RingBuffer.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace Visca {

class VISCA_EXPORT ViscaController {
public:
    explicit ViscaController(std::unique_ptr<ICommunicator> communicator);
    ~ViscaController();

    // No copy
    ViscaController(const ViscaController&) = delete;
    ViscaController& operator=(const ViscaController&) = delete;

    // Move allowed
    ViscaController(ViscaController&&) = default;
    ViscaController& operator=(ViscaController&&) = default;

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Synchronous command execution
    bool execute(const Command& cmd);
    bool execute(const Command& cmd, Response& response);

    // Asynchronous command execution
    bool sendAsync(const Command& cmd);
    bool pollResponse(Response& response, int timeoutMs = 0);

    // Inquiry helpers
    uint16_t getZoomPosition();
    uint16_t getFocusPosition();
    uint8_t getPowerStatus();

    // Camera address
    void setCameraAddress(uint8_t address) { m_address = address; }
    uint8_t cameraAddress() const { return m_address; }

    // Timeouts
    void setResponseTimeout(int milliseconds) { m_timeoutMs = milliseconds; }
    int responseTimeout() const { return m_timeoutMs; }

    struct VersionInfo {
        uint16_t vendorId;
        uint16_t modelId;
        uint32_t romRevision;
        uint8_t maxSocket;
    };
    VersionInfo getVersionInfo();

private:
    void receiveThread();
    bool waitForAck(int timeoutMs);
    bool waitForCompletion(int timeoutMs, Response& response);
    bool sendRaw(const std::vector<uint8_t>& data);

    std::unique_ptr<ICommunicator> m_communicator;
    uint8_t m_address { 1 };
    int m_timeoutMs { 1000 };

    std::atomic<bool> m_running { false };
    std::thread m_receiveThread;
    RingBuffer<std::vector<uint8_t>, 64> m_receiveBuffer;

    mutable std::mutex m_sendMutex;
    std::condition_variable m_responseCond;
    std::vector<uint8_t> m_pendingResponse;
    std::atomic<bool> m_responseAvailable { false };
};

}
