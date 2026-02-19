#include "Logger.h"
#include <chrono>
#include <cstdarg>
#include <cstring>
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

LogLevel Logger::getLevel() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentLevel;
}

void Logger::enableLogging(bool enable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loggingEnabled = enable;
}

bool Logger::isLoggingEnabled() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_loggingEnabled;
}

void Logger::setOutput(std::ostream* output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = output;
    m_useConsole = true;
}

bool Logger::setOutputFile(const std::string& filename, bool append)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try {
        auto fileStream = std::make_unique<std::ofstream>();
        std::ios_base::openmode mode = std::ios_base::out;
        if (append) {
            mode |= std::ios_base::app;
        }

        fileStream->open(filename, mode);
        if (fileStream->is_open()) {
            m_fileOutput = std::move(fileStream);
            m_useFile = true;
            m_useConsole = false;
            return true;
        }
    } catch (...) {
        // Handle any exceptions
    }

    return false;
}

void Logger::setOutputToConsole()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_useConsole = true;
    m_useFile = false;
    if (m_fileOutput) {
        m_fileOutput->close();
        m_fileOutput.reset();
    }
}

void Logger::setOutputToBoth(std::ostream* consoleOutput, const std::string& filename, bool append)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_consoleOutput = consoleOutput;
    m_useConsole = true;

    try {
        auto fileStream = std::make_unique<std::ofstream>();
        std::ios_base::openmode mode = std::ios_base::out;
        if (append) {
            mode |= std::ios_base::app;
        }

        fileStream->open(filename, mode);
        if (fileStream->is_open()) {
            m_fileOutput = std::move(fileStream);
            m_useFile = true;
        }
    } catch (...) {
        m_useFile = false;
    }
}

void Logger::closeFile()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileOutput) {
        m_fileOutput->close();
        m_fileOutput.reset();
    }
    m_useFile = false;
}

void Logger::enableLocationInfo(bool enable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_showLocation = enable;
}

bool Logger::isLocationInfoEnabled() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_showLocation;
}

bool Logger::shouldLog(LogLevel level) const
{
    // Check if logging is globally enabled and if the level meets the threshold
    return m_loggingEnabled && (level <= m_currentLevel);
}

void Logger::writeToOutputs(const std::string& formattedMessage)
{
    if (m_useConsole && m_consoleOutput) {
        *m_consoleOutput << formattedMessage;
    }

    if (m_useFile && m_fileOutput && m_fileOutput->is_open()) {
        *m_fileOutput << formattedMessage;
        m_fileOutput->flush();
    }
}

const char* Logger::extractFileName(const char* filePath)
{
    if (!filePath)
        return "";

    const char* lastSlash = strrchr(filePath, '/');
#ifdef _WIN32
    const char* lastBackslash = strrchr(filePath, '\\');
    if (lastBackslash > lastSlash)
        lastSlash = lastBackslash;
#endif
    return (lastSlash != nullptr) ? lastSlash + 1 : filePath;
}

void Logger::log(LogLevel level, const char* file, const char* function, int line, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(level))
        return;

    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&timeT);

    std::ostringstream formattedMessage;

    // Timestamp
    formattedMessage << "[" << std::put_time(&tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count()
                     << "]";

    // Log level
    formattedMessage << " [" << levelToString(level) << "]";

    // Location info (if enabled)
    if (m_showLocation && file && function) {
        formattedMessage << " [" << extractFileName(file) << ":" << line << " " << function << "]";
    }

    // Actual message
    formattedMessage << " " << message << std::endl;

    writeToOutputs(formattedMessage.str());
}

void Logger::log(LogLevel level, const char* file, const char* function, int line, const char* format, ...)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(level))
        return;

    char buffer[2048];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    log(level, file, function, line, std::string(buffer));
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
