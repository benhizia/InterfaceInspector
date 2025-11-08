#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace switchspy { namespace view {
struct MemberMetadata { std::string name; size_t offset; size_t size; size_t align; std::string doc; };
class MetadataParser {
public:
    bool load(const std::string& filename);
    const std::vector<MemberMetadata>& getMembers() const { return members_; }
private:
    std::vector<MemberMetadata> members_;
    nlohmann::json data_;
};
}} // namespace
