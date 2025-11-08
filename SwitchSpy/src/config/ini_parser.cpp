#include "switchspy/config/ini_parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace switchspy {
namespace config {

bool IniParser::parse(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    data_.clear();
    std::string current_section;
    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
        line_number++;

        if (!parseLine(line, current_section)) {
            // Skip invalid lines or continue based on error handling policy
        }
    }

    return true;
}

const IniParser::Section* IniParser::getSection(const std::string& name) const {
    auto it = data_.find(name);
    return (it != data_.end()) ? &it->second : nullptr;
}

std::string IniParser::getValue(const std::string& section, const std::string& key,
                                const std::string& default_value) const {
    const Section* sec = getSection(section);
    if (!sec) return default_value;

    auto it = sec->find(key);
    return (it != sec->end()) ? it->second : default_value;
}

int IniParser::getInt(const std::string& section, const std::string& key, int default_value) const {
    std::string value = getValue(section, key);
    if (value.empty()) return default_value;

    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

bool IniParser::getBool(const std::string& section, const std::string& key, bool default_value) const {
    std::string value = getValue(section, key);
    if (value.empty()) return default_value;

    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);

    if (lower_value == "true" || lower_value == "yes" || lower_value == "1" || lower_value == "on") {
        return true;
    } else if (lower_value == "false" || lower_value == "no" || lower_value == "0" || lower_value == "off") {
        return false;
    }

    return default_value;
}

bool IniParser::hasSection(const std::string& name) const {
    return data_.find(name) != data_.end();
}

std::vector<std::string> IniParser::getSectionNames(const std::string& prefix) const {
    std::vector<std::string> names;

    for (const auto& pair : data_) {
        if (prefix.empty() || pair.first.find(prefix) == 0) {
            names.push_back(pair.first);
        }
    }

    return names;
}

std::string IniParser::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool IniParser::parseLine(const std::string& line, std::string& current_section) {
    std::string trimmed = trim(line);

    // Skip empty lines and comments
    if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
        return true;
    }

    // Check for section header
    if (trimmed[0] == '[' && trimmed.back() == ']') {
        current_section = trimmed.substr(1, trimmed.length() - 2);
        current_section = trim(current_section);

        // Create section if it doesn't exist
        if (data_.find(current_section) == data_.end()) {
            data_[current_section] = Section();
        }

        return true;
    }

    // Parse key=value pair
    size_t equals_pos = trimmed.find('=');
    if (equals_pos != std::string::npos) {
        std::string key = trim(trimmed.substr(0, equals_pos));
        std::string value = trim(trimmed.substr(equals_pos + 1));

        if (!key.empty()) {
            // Add to current section (or global if no section specified)
            if (current_section.empty()) {
                current_section = "global";
                if (data_.find(current_section) == data_.end()) {
                    data_[current_section] = Section();
                }
            }

            data_[current_section][key] = value;
            return true;
        }
    }

    // Invalid line format
    return false;
}

} // namespace config
} // namespace switchspy
