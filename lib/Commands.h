#pragma once

#include "Export.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Visca {

class VISCA_EXPORT Command {
public:
    Command() = default;

    // Power
    static Command powerOn(uint8_t address = 1);
    static Command powerOff(uint8_t address = 1);
    static Command powerInquiry(uint8_t address = 1);

    // Zoom
    static Command zoomStop(uint8_t address = 1);
    static Command zoomTeleStandard(uint8_t address = 1);
    static Command zoomWideStandard(uint8_t address = 1);
    static Command zoomTeleVariable(uint8_t address, uint8_t speed);
    static Command zoomWideVariable(uint8_t address, uint8_t speed);
    static Command zoomDirect(uint8_t address, uint16_t position);
    static Command zoomPositionInquiry(uint8_t address = 1);

    // Focus
    static Command focusStop(uint8_t address = 1);
    static Command focusFarStandard(uint8_t address = 1);
    static Command focusNearStandard(uint8_t address = 1);
    static Command focusFarVariable(uint8_t address, uint8_t speed);
    static Command focusNearVariable(uint8_t address, uint8_t speed);
    static Command focusDirect(uint8_t address, uint16_t position);
    static Command focusAuto(uint8_t address = 1);
    static Command focusManual(uint8_t address = 1);
    static Command focusOnePushTrigger(uint8_t address = 1);
    static Command focusPositionInquiry(uint8_t address = 1);

    // Version inquiry
    static Command versionInquiry(uint8_t address = 1);

    const std::vector<uint8_t>& packet() const { return m_packet; }
    size_t size() const { return m_packet.size(); }
    bool empty() const { return m_packet.empty(); }

private:
    explicit Command(std::vector<uint8_t>&& packet);
    static Command create(uint8_t address, uint8_t category, uint8_t command, const std::vector<uint8_t>& params = {});

    std::vector<uint8_t> m_packet;
};

class VISCA_EXPORT Response {
public:
    Response() = default;

    bool parse(const std::vector<uint8_t>& data);

    bool isAcknowledge() const;
    bool isCompletion() const;
    bool isError() const;

    uint8_t socketNumber() const;
    uint8_t errorCode() const;
    std::string errorString() const;

    const std::vector<uint8_t>& data() const { return m_data; }

    template <typename T> T getValue(size_t offset = 0) const
    {
        T value = 0;
        size_t bytes = sizeof(T);
        for (size_t i = 0; i < bytes && (offset + i) < m_data.size(); ++i)
            value = (value << 8) | m_data[offset + i];
        return value;
    }

    uint16_t getZoomPosition() const;
    uint16_t getFocusPosition() const;
    uint8_t getPowerStatus() const;

private:
    enum class Type { Acknowledge, Completion, Error, Unknown };

    Type m_type { Type::Unknown };
    uint8_t m_socket { 0 };
    uint8_t m_errorCode { 0 };
    std::vector<uint8_t> m_data;
};

}
