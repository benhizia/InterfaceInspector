#include "switchspy/view/connection_tab.hpp"
#include "switchspy/serialization/packet_formatter.hpp"
#include "imgui.h"

namespace switchspy { namespace view {
ConnectionTab::ConnectionTab(std::shared_ptr<model::ConnectionStateModel> conn) : connection_(conn) {}

void ConnectionTab::render() {
    ImGui::Columns(2);
    renderTreeView();
    ImGui::NextColumn();
    renderHexView();
    ImGui::Columns(1);
    renderStatsPanel();
}

void ConnectionTab::renderTreeView() {
    ImGui::Text("Packet Tree View");
    ImGui::Separator();
    auto packets = connection_->getBuffer().getAll();
    for (size_t i = 0; i < packets.size(); ++i) {
        if (ImGui::TreeNode((void*)(intptr_t)i, "Packet #%zu (%zu bytes)", i, packets[i].size)) {
            ImGui::Text("Direction: %s", packets[i].direction == PacketDirection::AtoB ? "A->B" : "B->A");
            ImGui::TreePop();
        }
    }
}

void ConnectionTab::renderHexView() {
    ImGui::Text("Hex View");
    ImGui::Separator();
    auto packets = connection_->getBuffer().getAll();
    if (!packets.empty()) {
        auto hex = serialization::PacketFormatter::formatHex(packets.back().data);
        ImGui::TextUnformatted(hex.c_str());
    }
}

void ConnectionTab::renderStatsPanel() {
    const auto& stats = connection_->getStats();
    ImGui::Text("Statistics:");
    ImGui::Text("Packets A->B: %llu", stats.packets_sent_a_to_b);
    ImGui::Text("Packets B->A: %llu", stats.packets_sent_b_to_a);
    ImGui::Text("Bytes A->B: %llu", stats.bytes_sent_a_to_b);
    ImGui::Text("Bytes B->A: %llu", stats.bytes_sent_b_to_a);
}
}} // namespace
