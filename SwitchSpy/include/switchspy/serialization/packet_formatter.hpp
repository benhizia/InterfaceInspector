#pragma once
#include "switchspy/common/types.hpp"
namespace switchspy { namespace serialization {
class PacketFormatter {
public:
    static std::string formatHex(const byte_vector& data);
};
}} // namespace
