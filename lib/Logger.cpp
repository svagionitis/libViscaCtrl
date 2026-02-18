#include "Logger.h"
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <iomanip>

namespace Visca {

Logger& Logger::instance()
{
    static Logger s_instance;
    return s_instance;
}

Logger::Logger() { }

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentLevel = level;
}

void Logger::setOutput(std::ostream* output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_output = output;
}

void Logger::log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (level > m_currentLevel)
        return;

    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&timeT);

    *m_output << "[" << std::put_time(&tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count()
              << "] [" << levelToString(level) << "] " << message << std::endl;
}

void Logger::log(LogLevel level, const char* format, ...)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (level > m_currentLevel)
        return;

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    log(level, std::string(buffer));
}

const char* Logger::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Debug:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}

}
