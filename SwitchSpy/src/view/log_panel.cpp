#include "switchspy/view/log_panel.hpp"
#include "imgui.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace switchspy {
namespace view {

LogPanel::LogPanel()
    : auto_scroll_(true)
    , max_display_lines_(1000)
    , filter_level_(logging::LogLevel::Trace)
    , last_update_(std::chrono::system_clock::now())
{
}

void LogPanel::render() {
    ImGui::BeginChild("LogPanel", ImVec2(0, 0), true);

    // Toolbar
    if (ImGui::Button("Clear")) {
        clear();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &auto_scroll_);

    ImGui::SameLine();
    ImGui::Text("Filter:");
    ImGui::SameLine();

    const char* level_names[] = {"Trace", "Debug", "Info", "Warning", "Error", "Critical"};
    int current_level = static_cast<int>(filter_level_);
    if (ImGui::Combo("##FilterLevel", &current_level, level_names, 6)) {
        filter_level_ = static_cast<logging::LogLevel>(current_level);
    }

    ImGui::Separator();

    // Get recent log entries
    auto entries = logging::Logger::instance().getRecentEntries(max_display_lines_);

    // Display logs
    ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& entry : entries) {
        // Filter by level
        if (entry.level < filter_level_) {
            continue;
        }

        // Format timestamp
        std::string timestamp = formatTimestamp(entry.timestamp);

        // Get color for this level
        const float* color = getColorForLevel(entry.level);

        // Format line
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));

        ImGui::Text("[%s] [%s] [%s] %s",
                    timestamp.c_str(),
                    logging::levelToShortString(entry.level),
                    entry.category.c_str(),
                    entry.message.c_str());

        ImGui::PopStyleColor();
    }

    // Auto-scroll
    if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::EndChild();
}

const float* LogPanel::getColorForLevel(logging::LogLevel level) const {
    switch (level) {
        case logging::LogLevel::Trace:    return colors_.trace;
        case logging::LogLevel::Debug:    return colors_.debug;
        case logging::LogLevel::Info:     return colors_.info;
        case logging::LogLevel::Warning:  return colors_.warning;
        case logging::LogLevel::Error:    return colors_.error;
        case logging::LogLevel::Critical: return colors_.critical;
        default:                          return colors_.info;
    }
}

std::string LogPanel::formatTimestamp(const std::chrono::system_clock::time_point& tp) const {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()) % 1000;

    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << tm.tm_hour << ":"
        << std::setw(2) << tm.tm_min << ":"
        << std::setw(2) << tm.tm_sec << "."
        << std::setw(3) << ms.count();

    return oss.str();
}

void LogPanel::clear() {
    logging::Logger::instance().clearHistory();
}

// Console sink implementation
void ConsoleSink::write(const logging::LogEntry& entry) {
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;

    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif

    std::cout << "["
              << std::setfill('0')
              << std::setw(2) << tm.tm_hour << ":"
              << std::setw(2) << tm.tm_min << ":"
              << std::setw(2) << tm.tm_sec << "."
              << std::setw(3) << ms.count()
              << "] ["
              << logging::levelToShortString(entry.level)
              << "] ["
              << entry.category
              << "] "
              << entry.message
              << std::endl;
}

void ConsoleSink::flush() {
    std::cout.flush();
}

} // namespace view
} // namespace switchspy
