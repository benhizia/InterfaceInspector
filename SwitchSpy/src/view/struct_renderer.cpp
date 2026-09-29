#include "switchspy/view/struct_renderer.hpp"
#include "imgui.h"
namespace switchspy { namespace view {
StructRenderer::StructRenderer(const MetadataParser& parser) : parser_(parser) {}
void StructRenderer::render(const void* data) {
    for (const auto& member : parser_.getMembers()) {
        ImGui::Text("%s @ offset %zu (size: %zu)", member.name.c_str(), member.offset, member.size);
    }
}
}} // namespace
