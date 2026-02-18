#include "Commands.h"
#include <cstring>

namespace Visca {

Command::Command(std::vector<uint8_t>&& packet)
    : m_packet(std::move(packet))
{
}

Command Command::create(uint8_t address, uint8_t category, uint8_t command, const std::vector<uint8_t>& params)
{
    std::vector<uint8_t> packet;
    packet.reserve(3 + params.size() + 1);

    packet.push_back(0x80 | (address & 0x07));
    packet.push_back(0x01); // Command
    packet.push_back(category);
    packet.push_back(command);

    packet.insert(packet.end(), params.begin(), params.end());
    packet.push_back(0xFF);

    return Command(std::move(packet));
}

// Power commands
Command Command::powerOn(uint8_t address) { return create(address, 0x04, 0x00, { 0x02 }); }

Command Command::powerOff(uint8_t address) { return create(address, 0x04, 0x00, { 0x03 }); }

Command Command::powerInquiry(uint8_t address)
{
    std::vector<uint8_t> packet;
    packet.push_back(0x80 | (address & 0x07));
    packet.push_back(0x09); // Inquiry
    packet.push_back(0x04);
    packet.push_back(0x00);
    packet.push_back(0xFF);
    return Command(std::move(packet));
}

// Version inquiry
Command Command::versionInquiry(uint8_t address)
{
    std::vector<uint8_t> packet;
    packet.push_back(0x80 | (address & 0x07));
    packet.push_back(0x09); // Inquiry
    packet.push_back(0x00);
    packet.push_back(0x02);
    packet.push_back(0xFF);
    return Command(std::move(packet));
}

// Zoom commands
Command Command::zoomStop(uint8_t address) { return create(address, 0x04, 0x07, { 0x00 }); }

Command Command::zoomTeleStandard(uint8_t address) { return create(address, 0x04, 0x07, { 0x02 }); }

Command Command::zoomWideStandard(uint8_t address) { return create(address, 0x04, 0x07, { 0x03 }); }

Command Command::zoomTeleVariable(uint8_t address, uint8_t speed)
{
    return create(address, 0x04, 0x07, { static_cast<uint8_t>(0x20 | (speed & 0x07)) });
}

Command Command::zoomWideVariable(uint8_t address, uint8_t speed)
{
    return create(address, 0x04, 0x07, { static_cast<uint8_t>(0x30 | (speed & 0x07)) });
}

Command Command::zoomDirect(uint8_t address, uint16_t position)
{
    return create(address, 0x04, 0x47,
        { static_cast<uint8_t>((position >> 12) & 0x0F), static_cast<uint8_t>((position >> 8) & 0x0F),
            static_cast<uint8_t>((position >> 4) & 0x0F), static_cast<uint8_t>(position & 0x0F) });
}

Command Command::zoomPositionInquiry(uint8_t address)
{
    std::vector<uint8_t> packet;
    packet.push_back(0x80 | (address & 0x07));
    packet.push_back(0x09); // Inquiry
    packet.push_back(0x04);
    packet.push_back(0x47);
    packet.push_back(0xFF);
    return Command(std::move(packet));
}

// Focus commands
Command Command::focusStop(uint8_t address) { return create(address, 0x04, 0x08, { 0x00 }); }

Command Command::focusFarStandard(uint8_t address) { return create(address, 0x04, 0x08, { 0x02 }); }

Command Command::focusNearStandard(uint8_t address) { return create(address, 0x04, 0x08, { 0x03 }); }

Command Command::focusFarVariable(uint8_t address, uint8_t speed)
{
    return create(address, 0x04, 0x08, { static_cast<uint8_t>(0x20 | (speed & 0x07)) });
}

Command Command::focusNearVariable(uint8_t address, uint8_t speed)
{
    return create(address, 0x04, 0x08, { static_cast<uint8_t>(0x30 | (speed & 0x07)) });
}

Command Command::focusDirect(uint8_t address, uint16_t position)
{
    return create(address, 0x04, 0x48,
        { static_cast<uint8_t>((position >> 12) & 0x0F), static_cast<uint8_t>((position >> 8) & 0x0F),
            static_cast<uint8_t>((position >> 4) & 0x0F), static_cast<uint8_t>(position & 0x0F) });
}

Command Command::focusAuto(uint8_t address) { return create(address, 0x04, 0x38, { 0x02 }); }

Command Command::focusManual(uint8_t address) { return create(address, 0x04, 0x38, { 0x03 }); }

Command Command::focusOnePushTrigger(uint8_t address) { return create(address, 0x04, 0x18, { 0x01 }); }

Command Command::focusPositionInquiry(uint8_t address)
{
    std::vector<uint8_t> packet;
    packet.push_back(0x80 | (address & 0x07));
    packet.push_back(0x09); // Inquiry
    packet.push_back(0x04);
    packet.push_back(0x48);
    packet.push_back(0xFF);
    return Command(std::move(packet));
}

// Response parsing
bool Response::parse(const std::vector<uint8_t>& data)
{
    if (data.size() < 3 || data.back() != 0xFF)
        return false;

    m_data = data;

    uint8_t header = data[0];
    if ((header & 0xF0) != 0x90 && (header & 0xF0) != 0xA0)
        return false;

    uint8_t messageType = data[1] & 0xF0;

    if (messageType == 0x40) { // Acknowledge
        m_type = Type::Acknowledge;
        m_socket = data[1] & 0x0F;
    } else if (messageType == 0x50) { // Completion
        m_type = Type::Completion;
        m_socket = data[1] & 0x0F;
    } else if (messageType == 0x60) { // Error
        m_type = Type::Error;
        m_socket = data[1] & 0x0F;
        if (data.size() >= 4)
            m_errorCode = data[2];
    } else {
        m_type = Type::Unknown;
    }

    return true;
}

bool Response::isAcknowledge() const { return m_type == Type::Acknowledge; }

bool Response::isCompletion() const { return m_type == Type::Completion; }

bool Response::isError() const { return m_type == Type::Error; }

uint8_t Response::socketNumber() const { return m_socket; }

uint8_t Response::errorCode() const { return m_errorCode; }

std::string Response::errorString() const
{
    switch (m_errorCode) {
    case 0x01:
        return "Message length error";
    case 0x02:
        return "Syntax error";
    case 0x03:
        return "Command buffer full";
    case 0x04:
        return "Command cancelled";
    case 0x05:
        return "No socket";
    case 0x41:
        return "Command not executable";
    default:
        return "Unknown error";
    }
}

uint16_t Response::getZoomPosition() const
{
    if (m_data.size() < 7)
        return 0;
    return (m_data[2] << 12) | (m_data[3] << 8) | (m_data[4] << 4) | m_data[5];
}

uint16_t Response::getFocusPosition() const
{
    if (m_data.size() < 7)
        return 0;
    return (m_data[2] << 12) | (m_data[3] << 8) | (m_data[4] << 4) | m_data[5];
}

uint8_t Response::getPowerStatus() const
{
    if (m_data.size() < 4)
        return 0;
    return m_data[2];
}

}
