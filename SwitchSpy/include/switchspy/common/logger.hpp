#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

namespace switchspy {
namespace logging {

// Log levels
enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5
};

// Log entry
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string category;
    std::string message;

    LogEntry(LogLevel lvl, const std::string& cat, const std::string& msg)
        : timestamp(std::chrono::system_clock::now())
        , level(lvl)
        , category(cat)
        , message(msg)
    {}
};

// Abstract log sink interface
class ILogSink {
public:
    virtual ~ILogSink() = default;
    virtual void write(const LogEntry& entry) = 0;
    virtual void flush() = 0;
};

// Main logger class
class Logger {
public:
    static Logger& instance();

    // Add a log sink
    void addSink(std::shared_ptr<ILogSink> sink);

    // Remove all sinks
    void clearSinks();

    // Set minimum log level
    void setLevel(LogLevel level) { min_level_ = level; }
    LogLevel getLevel() const { return min_level_; }

    // Log methods
    void log(LogLevel level, const std::string& category, const std::string& message);

    void trace(const std::string& category, const std::string& message) {
        log(LogLevel::Trace, category, message);
    }

    void debug(const std::string& category, const std::string& message) {
        log(LogLevel::Debug, category, message);
    }

    void info(const std::string& category, const std::string& message) {
        log(LogLevel::Info, category, message);
    }

    void warning(const std::string& category, const std::string& message) {
        log(LogLevel::Warning, category, message);
    }

    void error(const std::string& category, const std::string& message) {
        log(LogLevel::Error, category, message);
    }

    void critical(const std::string& category, const std::string& message) {
        log(LogLevel::Critical, category, message);
    }

    // Format helpers
    template<typename... Args>
    void logf(LogLevel level, const std::string& category, const char* format, Args... args);

    // Get recent log entries (for GUI display)
    std::vector<LogEntry> getRecentEntries(size_t count = 1000) const;

    // Get all entries since a timestamp
    std::vector<LogEntry> getEntriesSince(std::chrono::system_clock::time_point since) const;

    // Clear history
    void clearHistory();

    // Flush all sinks
    void flush();

private:
    Logger();
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel min_level_;
    std::vector<std::shared_ptr<ILogSink>> sinks_;
    std::vector<LogEntry> history_;
    size_t max_history_size_;
    mutable std::mutex mutex_;
};

// Helper macros
#define LOG_TRACE(category, message) switchspy::logging::Logger::instance().trace(category, message)
#define LOG_DEBUG(category, message) switchspy::logging::Logger::instance().debug(category, message)
#define LOG_INFO(category, message) switchspy::logging::Logger::instance().info(category, message)
#define LOG_WARNING(category, message) switchspy::logging::Logger::instance().warning(category, message)
#define LOG_ERROR(category, message) switchspy::logging::Logger::instance().error(category, message)
#define LOG_CRITICAL(category, message) switchspy::logging::Logger::instance().critical(category, message)

// Utility functions
const char* levelToString(LogLevel level);
const char* levelToShortString(LogLevel level);

} // namespace logging
} // namespace switchspy
