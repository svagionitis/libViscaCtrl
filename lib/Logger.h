#pragma once

#include "Export.h"
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <fstream>
#include <memory>

namespace Visca {

enum class VISCA_EXPORT LogLevel { Error, Warning, Info, Debug };

class VISCA_EXPORT Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void setOutput(std::ostream* output);
    
    // New methods for file logging
    bool setOutputFile(const std::string& filename, bool append = true);
    void setOutputToConsole();
    void setOutputToBoth(std::ostream* consoleOutput, const std::string& filename, bool append = true);
    void closeFile();

    void log(LogLevel level, const std::string& message);
    void log(LogLevel level, const char* format, ...);

private:
    Logger();
    ~Logger() = default;

    static const char* levelToString(LogLevel level);
    
    // Helper method to write to all active outputs
    void writeToOutputs(const std::string& formattedMessage);

    std::mutex m_mutex;
    LogLevel m_currentLevel { LogLevel::Info };
    std::ostream* m_consoleOutput { &std::cout };
    std::unique_ptr<std::ofstream> m_fileOutput;
    bool m_useConsole { true };
    bool m_useFile { false };
};

#define VISCALOG_ERROR(msg)                                                                                            \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Error, oss.str());                                              \
    } while (0)

#define VISCALOG_WARN(msg)                                                                                             \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Warning, oss.str());                                            \
    } while (0)

#define VISCALOG_INFO(msg)                                                                                             \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Info, oss.str());                                               \
    } while (0)

#define VISCALOG_DEBUG(msg)                                                                                            \
    do {                                                                                                               \
        std::ostringstream oss;                                                                                        \
        oss << msg;                                                                                                    \
        Visca::Logger::instance().log(Visca::LogLevel::Debug, oss.str());                                              \
    } while (0)

}