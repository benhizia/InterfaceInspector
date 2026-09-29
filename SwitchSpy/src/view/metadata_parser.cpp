#include "switchspy/view/metadata_parser.hpp"
#include <fstream>

namespace switchspy { namespace view {
bool MetadataParser::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    try {
        file >> data_;
        // Parse members from first struct/class in JSON
        for (auto& [key, value] : data_.items()) {
            if (key == "system") continue;
            if (value.contains("members")) {
                for (auto& member : value["members"]) {
                    MemberMetadata meta;
                    meta.name = member["name"];
                    meta.offset = member["offset"];
                    meta.size = member["size"];
                    meta.align = member["align"];
                    meta.doc = member.value("doc", "");
                    members_.push_back(meta);
                }
                break;
            }
        }
        return true;
    } catch (...) { return false; }
}
}} // namespace
