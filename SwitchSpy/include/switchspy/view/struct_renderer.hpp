#pragma once
#include "switchspy/view/metadata_parser.hpp"
namespace switchspy { namespace view {
class StructRenderer {
public:
    explicit StructRenderer(const MetadataParser& parser);
    void render(const void* data);
private:
    const MetadataParser& parser_;
};
}} // namespace
