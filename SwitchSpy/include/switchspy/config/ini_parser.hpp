#pragma once

#include <string>
#include <map>
#include <vector>

namespace switchspy {
namespace config {

/**
 * Simple INI file parser
 * Parses INI files with sections and key=value pairs
 */
class IniParser {
public:
    using Section = std::map<std::string, std::string>;
    using IniData = std::map<std::string, Section>;

    /**
     * Parse INI file
     * @param filename Path to INI file
     * @return true if parsing succeeded
     */
    bool parse(const std::string& filename);

    /**
     * Get all sections
     */
    const IniData& getSections() const { return data_; }

    /**
     * Get section by name
     */
    const Section* getSection(const std::string& name) const;

    /**
     * Get value from section
     */
    std::string getValue(const std::string& section, const std::string& key,
                        const std::string& default_value = "") const;

    /**
     * Get integer value
     */
    int getInt(const std::string& section, const std::string& key, int default_value = 0) const;

    /**
     * Get boolean value
     */
    bool getBool(const std::string& section, const std::string& key, bool default_value = false) const;

    /**
     * Check if section exists
     */
    bool hasSection(const std::string& name) const;

    /**
     * Get all section names with a prefix
     */
    std::vector<std::string> getSectionNames(const std::string& prefix = "") const;

private:
    std::string trim(const std::string& str) const;
    bool parseLine(const std::string& line, std::string& current_section);

    IniData data_;
};

} // namespace config
} // namespace switchspy
