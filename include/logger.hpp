#pragma once

#include <fstream>
#include <mutex>
#include <string>

// Severity levels, ordered from most to least verbose.
enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

// Thread-safe singleton logger.
//
// Every record is written to the console (stdout for < WARN, stderr otherwise)
// and, if configured, to a log file. A single mutex serialises writes so log
// lines coming from many worker threads never interleave.
class Logger {
public:
    static Logger& instance();

    // Open the log file (truncating it) and set the minimum level that will be
    // emitted. Passing an empty path disables file output (console only).
    void init(const std::string& filePath, LogLevel minLevel = LogLevel::INFO);

    void log(LogLevel level, const std::string& message);

    void debug(const std::string& m) { log(LogLevel::DEBUG, m); }
    void info(const std::string& m)  { log(LogLevel::INFO, m); }
    void warn(const std::string& m)  { log(LogLevel::WARN, m); }
    void error(const std::string& m) { log(LogLevel::ERROR, m); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;

    std::mutex mutex_;
    std::ofstream file_;
    LogLevel minLevel_ = LogLevel::INFO;
};
