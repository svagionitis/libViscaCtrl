#pragma once

#include "Export.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace Visca {

enum class VISCA_EXPORT LogLevel { Error, Warning, Info, Debug };

class VISCA_EXPORT Logger {
public:
    static Logger& instance();

    // Level management
    void setLevel(LogLevel level);
    LogLevel getLevel() const;

    // Global enable/disable
    void enableLogging(bool enable);
    bool isLoggingEnabled() const;

    // Output configuration
    void setOutput(std::ostream* output);
    bool setOutputFile(const std::string& filename, bool append = true);
    void setOutputToConsole();
    void setOutputToBoth(std::ostream* consoleOutput, const std::string& filename, bool append = true);
    void closeFile();

    // Core logging methods with location info
    void log(LogLevel level, const char* file, const char* function, int line, const std::string& message);
    void log(LogLevel level, const char* file, const char* function, int line, const char* format, ...);

    // Configuration
    void enableLocationInfo(bool enable);
    bool isLocationInfoEnabled() const;

private:
    Logger();
    ~Logger() = default;

    static const char* levelToString(LogLevel level);

    // Helper method to write to all active outputs
    void writeToOutputs(const std::string& formattedMessage);

    // Helper to extract filename from path
    static const char* extractFileName(const char* filePath);

    // Helper to check if logging should occur
    bool shouldLog(LogLevel level) const;

    mutable std::mutex m_mutex;
    LogLevel m_currentLevel { LogLevel::Info };
    std::ostream* m_consoleOutput { &std::cout };
    std::unique_ptr<std::ofstream> m_fileOutput;
    bool m_useConsole { true };
    bool m_useFile { false };
    bool m_showLocation { true };
    bool m_loggingEnabled { true }; // Global logging switch
};

// Updated macros that check if logging is enabled
#define VISCALOG_ERROR(msg)                                                                                            \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled()) {                                                            \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, oss.str());        \
        }                                                                                                              \
    } while (0)

#define VISCALOG_WARN(msg)                                                                                             \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled()) {                                                            \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Warning, __FILE__, __FUNCTION__, __LINE__, oss.str());      \
        }                                                                                                              \
    } while (0)

#define VISCALOG_INFO(msg)                                                                                             \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled()) {                                                            \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, oss.str());         \
        }                                                                                                              \
    } while (0)

#define VISCALOG_DEBUG(msg)                                                                                            \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled()) {                                                            \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, oss.str());        \
        }                                                                                                              \
    } while (0)

// Conditional logging macros
#define VISCALOG_ERROR_IF(condition, msg)                                                                              \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled() && (condition)) {                                             \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, oss.str());        \
        }                                                                                                              \
    } while (0)

#define VISCALOG_WARN_IF(condition, msg)                                                                               \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled() && (condition)) {                                             \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Warning, __FILE__, __FUNCTION__, __LINE__, oss.str());      \
        }                                                                                                              \
    } while (0)

#define VISCALOG_INFO_IF(condition, msg)                                                                               \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled() && (condition)) {                                             \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Info, __FILE__, __FUNCTION__, __LINE__, oss.str());         \
        }                                                                                                              \
    } while (0)

#define VISCALOG_DEBUG_IF(condition, msg)                                                                              \
    do {                                                                                                               \
        if (Visca::Logger::instance().isLoggingEnabled() && (condition)) {                                             \
            std::ostringstream oss;                                                                                    \
            oss << msg;                                                                                                \
            Visca::Logger::instance().log(Visca::LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__, oss.str());        \
        }                                                                                                              \
    } while (0)

// Macros that always log regardless of global setting (useful for critical errors)
#define VISCALOG_FORCE_ERROR(msg)                                                                                      \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Error, __FILE__, __FUNCTION__, __LINE__, oss.str());            \
    } while (0)

#define VISCALOG_FORCE_WARN(msg)                                                                                       \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Warning, __FILE__, __FUNCTION__, __LINE__, oss.str());          \
    } while (0)

}
