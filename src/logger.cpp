#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

namespace {

std::string timestamp() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto t = system_clock::to_time_t(now);
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string threadIdStr() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
    }
    return "?????";
}

}  // namespace

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

void Logger::init(const std::string& filePath, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = minLevel;
    if (!filePath.empty()) {
        file_.open(filePath, std::ios::out | std::ios::trunc);
        if (!file_) {
            std::cerr << "WARN: could not open log file: " << filePath << std::endl;
        }
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel_) {
        return;
    }

    std::ostringstream line;
    line << '[' << timestamp() << "] "
         << '[' << levelName(level) << "] "
         << "[tid " << threadIdStr() << "] "
         << message;
    const std::string text = line.str();

    std::lock_guard<std::mutex> lock(mutex_);
    if (level >= LogLevel::WARN) {
        std::cerr << text << std::endl;
    } else {
        std::cout << text << std::endl;
    }
    if (file_.is_open()) {
        file_ << text << '\n';
        file_.flush();
    }
}
