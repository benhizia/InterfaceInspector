#pragma once

#include "switchspy/common/logger.hpp"
#include <string>

namespace switchspy {
namespace view {

/**
 * ImGui log panel widget
 * Displays color-coded log messages with filtering
 */
class LogPanel {
public:
    LogPanel();

    // Render the log panel
    void render();

    // Configuration
    void setAutoScroll(bool enable) { auto_scroll_ = enable; }
    void setMaxDisplayLines(size_t max) { max_display_lines_ = max; }
    void setFilterLevel(logging::LogLevel level) { filter_level_ = level; }

    // Clear displayed logs
    void clear();

private:
    struct ColorScheme {
        float trace[4]    = {0.5f, 0.5f, 0.5f, 1.0f};  // Gray
        float debug[4]    = {0.7f, 0.7f, 1.0f, 1.0f};  // Light blue
        float info[4]     = {1.0f, 1.0f, 1.0f, 1.0f};  // White
        float warning[4]  = {1.0f, 0.9f, 0.3f, 1.0f};  // Yellow
        float error[4]    = {1.0f, 0.4f, 0.4f, 1.0f};  // Red
        float critical[4] = {1.0f, 0.0f, 0.0f, 1.0f};  // Bright red
    };

    const float* getColorForLevel(logging::LogLevel level) const;
    std::string formatTimestamp(const std::chrono::system_clock::time_point& tp) const;

    bool auto_scroll_;
    size_t max_display_lines_;
    logging::LogLevel filter_level_;
    ColorScheme colors_;
    std::chrono::system_clock::time_point last_update_;
};

/**
 * Console log sink - writes to stdout
 */
class ConsoleSink : public logging::ILogSink {
public:
    void write(const logging::LogEntry& entry) override;
    void flush() override;
};

} // namespace view
} // namespace switchspy
