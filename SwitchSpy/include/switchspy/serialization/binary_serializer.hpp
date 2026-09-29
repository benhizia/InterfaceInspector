#pragma once
#include "switchspy/common/types.hpp"
#include <fstream>

namespace switchspy { namespace serialization {
class BinarySerializer {
public:
    bool open(const std::string& filename);
    bool writePacket(const PacketInfo& packet);
    void close();
private:
    std::ofstream file_;
};
}} // namespace
