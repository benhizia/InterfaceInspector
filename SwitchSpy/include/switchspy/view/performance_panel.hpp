#pragma once

#include "switchspy/common/benchmark.hpp"
#include <string>

namespace switchspy {
namespace view {

/**
 * ImGui performance monitoring panel
 * Displays benchmarking data and highlights slow operations
 */
class PerformancePanel {
public:
    PerformancePanel();

    // Render the panel
    void render();

    // Configuration
    void setShowOnlySlowOps(bool show) { show_only_slow_ops_ = show; }
    void setSlowThreshold(uint64_t threshold_us) { slow_threshold_us_ = threshold_us; }

    // Reset all metrics
    void resetMetrics();

private:
    void renderMetricsTable();
    void renderSummary();
    const float* getColorForTime(uint64_t time_us, uint64_t threshold_us) const;

    bool show_only_slow_ops_;
    uint64_t slow_threshold_us_;

    struct ColorScheme {
        float good[4]     = {0.3f, 1.0f, 0.3f, 1.0f};  // Green
        float warning[4]  = {1.0f, 0.9f, 0.3f, 1.0f};  // Yellow
        float critical[4] = {1.0f, 0.4f, 0.4f, 1.0f};  // Red
    };

    ColorScheme colors_;
};

} // namespace view
} // namespace switchspy
