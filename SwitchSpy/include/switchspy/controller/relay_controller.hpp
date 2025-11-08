#pragma once
#include "switchspy/core/network_connection.hpp"
namespace switchspy { namespace controller {
class RelayController {
public:
    void startRelay(const std::string& name);
    void stopRelay(const std::string& name);
};
}} // namespace
