#pragma once
#include "switchspy/model/connection_state.hpp"
#include <memory>

namespace switchspy { namespace view {
class ConnectionTab {
public:
    explicit ConnectionTab(std::shared_ptr<model::ConnectionStateModel> conn);
    void render();
private:
    void renderTreeView();
    void renderHexView();
    void renderStatsPanel();
    std::shared_ptr<model::ConnectionStateModel> connection_;
};
}} // namespace
