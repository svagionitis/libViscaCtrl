#include "ViscaController.h"
#include "Logger.h"
#include <chrono>

namespace Visca {

ViscaController::ViscaController(std::unique_ptr<ICommunicator> communicator)
    : m_communicator(std::move(communicator))
{
}

ViscaController::~ViscaController() { disconnect(); }

bool ViscaController::connect()
{
    if (!m_communicator || !m_communicator->open()) {
        VISCALOG_ERROR("Failed to open communicator");
        return false;
    }

    m_running = true;
    m_receiveThread = std::thread(&ViscaController::receiveThread, this);

    VISCALOG_INFO("Connected to camera");
    return true;
}

void ViscaController::disconnect()
{
    m_running = false;
    if (m_receiveThread.joinable())
        m_receiveThread.join();

    if (m_communicator)
        m_communicator->close();

    VISCALOG_INFO("Disconnected from camera");
}

bool ViscaController::isConnected() const { return m_communicator && m_communicator->isOpen(); }

bool ViscaController::sendRaw(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(m_sendMutex);

    VISCALOG_DEBUG("Sending: " << std::hex);
    for (auto byte : data)
        VISCALOG_DEBUG(" " << std::hex << static_cast<int>(byte));

    return m_communicator->send(data);
}

bool ViscaController::execute(const Command& cmd)
{
    Response response;
    return execute(cmd, response);
}

bool ViscaController::execute(const Command& cmd, Response& response)
{
    if (!isConnected()) {
        VISCALOG_ERROR("Not connected");
        return false;
    }

    if (!sendRaw(cmd.packet()))
        return false;

    // Wait for acknowledge
    if (!waitForAck(m_timeoutMs)) {
        VISCALOG_ERROR("No acknowledge received");
        return false;
    }

    // Wait for completion
    if (!waitForCompletion(m_timeoutMs, response)) {
        VISCALOG_ERROR("No completion received");
        return false;
    }

    return !response.isError();
}

bool ViscaController::sendAsync(const Command& cmd)
{
    if (!isConnected()) {
        VISCALOG_ERROR("Not connected");
        return false;
    }

    return sendRaw(cmd.packet());
}

bool ViscaController::pollResponse(Response& response, int timeoutMs)
{
    auto startTime = std::chrono::steady_clock::now();

    while (m_running) {
        std::vector<uint8_t> data;
        if (m_receiveBuffer.pop(data)) {
            if (response.parse(data))
                return true;
        }

        if (timeoutMs >= 0) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
            if (elapsed.count() >= timeoutMs)
                break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
}

bool ViscaController::waitForAck(int timeoutMs)
{
    auto startTime = std::chrono::steady_clock::now();

    while (m_running) {
        Response response;
        if (pollResponse(response, 10)) {
            if (response.isAcknowledge())
                return true;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        if (elapsed.count() >= timeoutMs)
            break;
    }

    return false;
}

bool ViscaController::waitForCompletion(int timeoutMs, Response& response)
{
    auto startTime = std::chrono::steady_clock::now();

    while (m_running) {
        if (pollResponse(response, 10)) {
            if (response.isCompletion() || response.isError())
                return true;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        if (elapsed.count() >= timeoutMs)
            break;
    }

    return false;
}

void ViscaController::receiveThread()
{
    std::vector<uint8_t> buffer(256);

    while (m_running) {
        if (!m_communicator || !m_communicator->isOpen()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        size_t bytesRead = m_communicator->receive(buffer.data(), buffer.size());
        if (bytesRead > 0) {
            std::vector<uint8_t> data(buffer.begin(), buffer.begin() + bytesRead);
            VISCALOG_DEBUG("Received: " << bytesRead << " bytes");
            m_receiveBuffer.push(std::move(data));
            m_responseCond.notify_one();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

uint16_t ViscaController::getZoomPosition()
{
    Response response;
    if (execute(Command::zoomPositionInquiry(m_address), response))
        return response.getZoomPosition();
    return 0;
}

uint16_t ViscaController::getFocusPosition()
{
    Response response;
    if (execute(Command::focusPositionInquiry(m_address), response))
        return response.getFocusPosition();
    return 0;
}

uint8_t ViscaController::getPowerStatus()
{
    Response response;
    if (execute(Command::powerInquiry(m_address), response))
        return response.getPowerStatus();
    return 0;
}

ViscaController::VersionInfo ViscaController::getVersionInfo()
{
    VersionInfo info = { 0, 0, 0, 0 };

    Response response;
    if (execute(Command::versionInquiry(m_address), response)) {
        if (response.data().size() >= 8) {
            info.vendorId = (response.data()[2] << 8) | response.data()[3];
            info.modelId = (response.data()[4] << 8) | response.data()[5];
            info.romRevision = (response.data()[6] << 8) | response.data()[7];
            if (response.data().size() >= 9)
                info.maxSocket = response.data()[8];
        }
    }

    return info;
}

}
