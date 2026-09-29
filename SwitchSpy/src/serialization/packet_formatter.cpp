#include "switchspy/serialization/packet_formatter.hpp"
#include <sstream>
#include <iomanip>
namespace switchspy { namespace serialization {
std::string PacketFormatter::formatHex(const byte_vector& data) {
    std::stringstream ss;
    for (size_t i = 0; i < data.size(); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        if ((i + 1) % 16 == 0) ss << "\n";
        else ss << " ";
    }
    return ss.str();
}
}} // namespace
