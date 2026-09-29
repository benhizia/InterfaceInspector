#include "switchspy/model/connection_state.hpp"

namespace switchspy {
namespace model {

ConnectionStateModel::ConnectionStateModel(const std::string& name)
    : name_(name), state_(ConnectionState::Disconnected) {
    stats_.start_time = std::chrono::system_clock::now();
}

} // namespace model
} // namespace switchspy
