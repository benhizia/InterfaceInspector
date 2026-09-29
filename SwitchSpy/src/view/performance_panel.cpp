#include "switchspy/view/performance_panel.hpp"
#include "imgui.h"
#include <algorithm>

namespace switchspy {
namespace view {

PerformancePanel::PerformancePanel()
    : show_only_slow_ops_(false)
    , slow_threshold_us_(1000)  // 1ms default
{
}

void PerformancePanel::render() {
    ImGui::BeginChild("PerformancePanel", ImVec2(0, 0), true);

    // Toolbar
    if (ImGui::Button("Reset All Metrics")) {
        resetMetrics();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Show Only Slow Operations", &show_only_slow_ops_);

    ImGui::SameLine();
    ImGui::Text("Slow Threshold:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    int threshold_ms = slow_threshold_us_ / 1000;
    if (ImGui::InputInt("ms##SlowThreshold", &threshold_ms)) {
        slow_threshold_us_ = threshold_ms * 1000;
    }

    ImGui::Separator();

    // Summary
    renderSummary();

    ImGui::Separator();

    // Metrics table
    renderMetricsTable();

    ImGui::EndChild();
}

void PerformancePanel::renderSummary() {
    auto& monitor = benchmark::PerformanceMonitor::instance();
    auto all_metrics = monitor.getAllMetrics();

    uint64_t total_operations = 0;
    uint64_t slow_operations = 0;

    for (const auto& [name, metrics] : all_metrics) {
        total_operations += metrics.call_count;
        if (metrics.getAverageUs() > slow_threshold_us_) {
            slow_operations++;
        }
    }

    ImGui::Text("Total Operation Types: %zu", all_metrics.size());
    ImGui::SameLine(250);
    ImGui::Text("Total Calls: %llu", (unsigned long long)total_operations);

    if (slow_operations > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colors_.critical[0], colors_.critical[1],
                                                      colors_.critical[2], colors_.critical[3]));
        ImGui::Text("Slow Operations: %llu", (unsigned long long)slow_operations);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("Slow Operations: 0");
    }
}

void PerformancePanel::renderMetricsTable() {
    auto& monitor = benchmark::PerformanceMonitor::instance();
    auto all_metrics = monitor.getAllMetrics();

    if (all_metrics.empty()) {
        ImGui::TextDisabled("No performance data collected yet");
        return;
    }

    // Convert to vector for sorting
    std::vector<std::pair<std::string, benchmark::PerformanceMetrics>> sorted_metrics(
        all_metrics.begin(), all_metrics.end());

    // Sort by average time (descending)
    std::sort(sorted_metrics.begin(), sorted_metrics.end(),
              [](const auto& a, const auto& b) {
                  return a.second.getAverageUs() > b.second.getAverageUs();
              });

    // Table
    if (ImGui::BeginTable("PerformanceMetrics", 7,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {

        // Headers
        ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Last (μs)", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Avg (μs)", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Min (μs)", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Max (μs)", ImGuiTableColumnFlags_WidthFixed, 80);
        ImGui::TableSetupColumn("Total (ms)", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Rows
        for (const auto& [name, metrics] : sorted_metrics) {
            // Filter if needed
            if (show_only_slow_ops_ && metrics.getAverageUs() <= slow_threshold_us_) {
                continue;
            }

            ImGui::TableNextRow();

            // Operation name
            ImGui::TableNextColumn();
            ImGui::Text("%s", name.c_str());

            // Calls
            ImGui::TableNextColumn();
            ImGui::Text("%llu", (unsigned long long)metrics.call_count);

            // Determine threshold for this operation
            uint64_t threshold = slow_threshold_us_;
            if (name.find("network") == 0) {
                threshold = benchmark::thresholds::NETWORK_IO;
            } else if (name.find("relay") == 0) {
                threshold = benchmark::thresholds::RELAY_LATENCY;
            } else if (name.find("buffer") == 0) {
                threshold = benchmark::thresholds::BUFFER_OPERATION;
            } else if (name.find("gui") == 0) {
                threshold = benchmark::thresholds::GUI_FRAME;
            }

            // Last time
            ImGui::TableNextColumn();
            const float* color = getColorForTime(metrics.last_time_us, threshold);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));
            ImGui::Text("%llu", (unsigned long long)metrics.last_time_us);
            ImGui::PopStyleColor();

            // Average time
            ImGui::TableNextColumn();
            uint64_t avg_us = static_cast<uint64_t>(metrics.getAverageUs());
            color = getColorForTime(avg_us, threshold);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));
            ImGui::Text("%llu", (unsigned long long)avg_us);
            ImGui::PopStyleColor();

            // Min time
            ImGui::TableNextColumn();
            ImGui::Text("%llu", (unsigned long long)metrics.min_time_us);

            // Max time
            ImGui::TableNextColumn();
            color = getColorForTime(metrics.max_time_us, threshold);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color[0], color[1], color[2], color[3]));
            ImGui::Text("%llu", (unsigned long long)metrics.max_time_us);
            ImGui::PopStyleColor();

            // Total time
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", metrics.total_time_us / 1000.0);
        }

        ImGui::EndTable();
    }
}

const float* PerformancePanel::getColorForTime(uint64_t time_us, uint64_t threshold_us) const {
    if (time_us > threshold_us * 2) {
        return colors_.critical;
    } else if (time_us > threshold_us) {
        return colors_.warning;
    } else {
        return colors_.good;
    }
}

void PerformancePanel::resetMetrics() {
    benchmark::PerformanceMonitor::instance().resetAll();
}

} // namespace view
} // namespace switchspy
