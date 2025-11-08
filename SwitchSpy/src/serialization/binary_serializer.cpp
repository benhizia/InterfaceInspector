#include "switchspy/serialization/binary_serializer.hpp"

namespace switchspy { namespace serialization {
bool BinarySerializer::open(const std::string& filename) {
    file_.open(filename, std::ios::binary);
    return file_.is_open();
}
bool BinarySerializer::writePacket(const PacketInfo& packet) { return false; }
void BinarySerializer::close() { if (file_.is_open()) file_.close(); }
}} // namespace
