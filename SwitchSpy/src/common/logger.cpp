#include "switchspy/common/logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace switchspy {
namespace logging {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : min_level_(LogLevel::Info)
    , max_history_size_(10000)
{
}

void Logger::addSink(std::shared_ptr<ILogSink> sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.push_back(sink);
}

void Logger::clearSinks() {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.clear();
}

void Logger::log(LogLevel level, const std::string& category, const std::string& message) {
    if (level < min_level_) {
        return;
    }

    LogEntry entry(level, category, message);

    std::lock_guard<std::mutex> lock(mutex_);

    // Add to history
    history_.push_back(entry);

    // Trim history if needed
    if (history_.size() > max_history_size_) {
        history_.erase(history_.begin(), history_.begin() + (history_.size() - max_history_size_));
    }

    // Write to all sinks
    for (auto& sink : sinks_) {
        sink->write(entry);
    }
}

std::vector<LogEntry> Logger::getRecentEntries(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (history_.size() <= count) {
        return history_;
    }

    return std::vector<LogEntry>(history_.end() - count, history_.end());
}

std::vector<LogEntry> Logger::getEntriesSince(std::chrono::system_clock::time_point since) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<LogEntry> result;
    for (const auto& entry : history_) {
        if (entry.timestamp >= since) {
            result.push_back(entry);
        }
    }

    return result;
}

void Logger::clearHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    history_.clear();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& sink : sinks_) {
        sink->flush();
    }
}

const char* levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARNING";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

const char* levelToShortString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return "TRC";
        case LogLevel::Debug:    return "DBG";
        case LogLevel::Info:     return "INF";
        case LogLevel::Warning:  return "WRN";
        case LogLevel::Error:    return "ERR";
        case LogLevel::Critical: return "CRT";
        default:                 return "???";
    }
}

} // namespace logging
} // namespace switchspy
