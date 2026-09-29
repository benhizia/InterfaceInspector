#pragma once
#include "switchspy/common/types.hpp"
namespace switchspy { namespace serialization {
class BinaryDeserializer {
public:
    bool open(const std::string& filename) { return false; }
    bool readPacket(PacketInfo& packet) { return false; }
};
}} // namespace
