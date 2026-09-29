#pragma once

#include "switchspy/config/port_mapping.hpp"
#include "switchspy/common/types.hpp"
#include <map>
#include <string>
#include <memory>

namespace switchspy {
namespace config {

/**
 * Global configuration manager
 * Loads and manages application configuration from INI files
 */
class ConfigManager {
public:
    static ConfigManager& instance();

    // Load configuration from file
    bool loadFromFile(const std::string& filename);

    // Save configuration to file
    bool saveToFile(const std::string& filename) const;

    // Global settings
    ConnectionMode getMode() const { return mode_; }
    void setMode(ConnectionMode mode) { mode_ = mode; }

    VisualizationMode getVisualizationMode() const { return visualization_mode_; }
    void setVisualizationMode(VisualizationMode mode) { visualization_mode_ = mode; }

    std::string getTheme() const { return theme_; }
    void setTheme(const std::string& theme) { theme_ = theme; }

    // Recording settings
    bool isRecordingEnabled() const { return recording_enabled_; }
    void setRecordingEnabled(bool enabled) { recording_enabled_ = enabled; }

    std::string getOutputDir() const { return output_dir_; }
    void setOutputDir(const std::string& dir) { output_dir_ = dir; }

    bool getAutoSave() const { return auto_save_; }
    void setAutoSave(bool enable) { auto_save_ = enable; }

    size_t getBufferSize() const { return buffer_size_; }
    void setBufferSize(size_t size) { buffer_size_ = size; }

    // Port mappings
    void addPortMapping(const PortMapping& mapping);
    const std::map<std::string, PortMapping>& getPortMappings() const { return port_mappings_; }
    const PortMapping* getPortMapping(const std::string& name) const;

    // Replay settings
    struct ReplayConfig {
        std::string file;
        std::string mode;  // "multicast_emit", "tcp_emit", etc.
        std::string target_group;
        uint16_t target_port;

        ReplayConfig() : target_port(0) {}
    };

    void addReplayConfig(const std::string& name, const ReplayConfig& config);
    const std::map<std::string, ReplayConfig>& getReplayConfigs() const { return replay_configs_; }

private:
    ConfigManager();
    ~ConfigManager() = default;

    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Parse connection section
    void parseConnectionSection(const std::string& section, const std::map<std::string, std::string>& values);

    // Parse replay section
    void parseReplaySection(const std::string& section, const std::map<std::string, std::string>& values);

    // Global settings
    ConnectionMode mode_;
    VisualizationMode visualization_mode_;
    std::string theme_;

    // Recording settings
    bool recording_enabled_;
    std::string output_dir_;
    bool auto_save_;
    size_t buffer_size_;

    // Port mappings
    std::map<std::string, PortMapping> port_mappings_;

    // Replay configurations
    std::map<std::string, ReplayConfig> replay_configs_;
};

} // namespace config
} // namespace switchspy
