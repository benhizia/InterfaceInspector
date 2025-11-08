#include "switchspy/config/config_manager.hpp"
#include "switchspy/config/ini_parser.hpp"
#include <iostream>

namespace switchspy {
namespace config {

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
    : mode_(ConnectionMode::Relay)
    , visualization_mode_(VisualizationMode::Deferred)
    , theme_("dark")
    , recording_enabled_(false)
    , output_dir_("./recordings")
    , auto_save_(true)
    , buffer_size_(10 * 1024 * 1024)  // 10MB default
{
}

bool ConfigManager::loadFromFile(const std::string& filename) {
    IniParser parser;

    if (!parser.parse(filename)) {
        std::cerr << "Failed to parse config file: " << filename << std::endl;
        return false;
    }

    // Parse global settings
    std::string mode_str = parser.getValue("global", "mode", "relay");
    if (mode_str == "relay") mode_ = ConnectionMode::Relay;
    else if (mode_str == "record") mode_ = ConnectionMode::Record;
    else if (mode_str == "replay") mode_ = ConnectionMode::Replay;

    std::string viz_str = parser.getValue("global", "visualization", "deferred");
    if (viz_str == "realtime") visualization_mode_ = VisualizationMode::Realtime;
    else visualization_mode_ = VisualizationMode::Deferred;

    theme_ = parser.getValue("global", "theme", "dark");

    // Parse recording settings
    recording_enabled_ = parser.getBool("recording", "enabled", false);
    output_dir_ = parser.getValue("recording", "output_dir", "./recordings");
    auto_save_ = parser.getBool("recording", "auto_save", true);
    buffer_size_ = parser.getInt("recording", "buffer_size", 10485760);

    // Parse connection sections
    auto connection_sections = parser.getSectionNames("connection.");
    for (const auto& section : connection_sections) {
        const auto* sec_data = parser.getSection(section);
        if (sec_data) {
            parseConnectionSection(section, *sec_data);
        }
    }

    // Parse replay sections
    auto replay_sections = parser.getSectionNames("replay.");
    for (const auto& section : replay_sections) {
        const auto* sec_data = parser.getSection(section);
        if (sec_data) {
            parseReplaySection(section, *sec_data);
        }
    }

    return true;
}

void ConfigManager::parseConnectionSection(const std::string& section,
                                           const std::map<std::string, std::string>& values) {
    // Extract connection name from "connection.name"
    std::string name = section.substr(11);  // Remove "connection." prefix

    PortMapping mapping;
    mapping.name = name;

    // Parse type
    auto type_it = values.find("type");
    if (type_it != values.end()) {
        if (type_it->second == "tcp") mapping.type = ConnectionType::TCP;
        else if (type_it->second == "udp") mapping.type = ConnectionType::UDP;
        else if (type_it->second == "multicast") mapping.type = ConnectionType::Multicast;
    }

    // Parse addresses
    auto listen_port_it = values.find("listen_port");
    if (listen_port_it != values.end()) {
        mapping.listen_port = std::stoi(listen_port_it->second);
    }

    auto forward_host_it = values.find("forward_host");
    if (forward_host_it != values.end()) {
        mapping.forward_host = forward_host_it->second;
    }

    auto forward_port_it = values.find("forward_port");
    if (forward_port_it != values.end()) {
        mapping.forward_port = std::stoi(forward_port_it->second);
    }

    auto multicast_group_it = values.find("multicast_group");
    if (multicast_group_it != values.end()) {
        mapping.multicast_group = multicast_group_it->second;
    }

    auto multicast_port_it = values.find("multicast_port");
    if (multicast_port_it != values.end()) {
        mapping.listen_port = std::stoi(multicast_port_it->second);
    }

    auto metadata_file_it = values.find("metadata_file");
    if (metadata_file_it != values.end()) {
        mapping.metadata_file = metadata_file_it->second;
    }

    if (mapping.isValid()) {
        port_mappings_[name] = mapping;
    }
}

void ConfigManager::parseReplaySection(const std::string& section,
                                       const std::map<std::string, std::string>& values) {
    std::string name = section.substr(7);  // Remove "replay." prefix

    ReplayConfig config;

    auto file_it = values.find("file");
    if (file_it != values.end()) config.file = file_it->second;

    auto mode_it = values.find("mode");
    if (mode_it != values.end()) config.mode = mode_it->second;

    auto group_it = values.find("target_group");
    if (group_it != values.end()) config.target_group = group_it->second;

    auto port_it = values.find("target_port");
    if (port_it != values.end()) config.target_port = std::stoi(port_it->second);

    replay_configs_[name] = config;
}

bool ConfigManager::saveToFile(const std::string& filename) const {
    // TODO: Implement INI file writing
    return false;
}

void ConfigManager::addPortMapping(const PortMapping& mapping) {
    port_mappings_[mapping.name] = mapping;
}

const PortMapping* ConfigManager::getPortMapping(const std::string& name) const {
    auto it = port_mappings_.find(name);
    return (it != port_mappings_.end()) ? &it->second : nullptr;
}

void ConfigManager::addReplayConfig(const std::string& name, const ReplayConfig& config) {
    replay_configs_[name] = config;
}

} // namespace config
} // namespace switchspy
