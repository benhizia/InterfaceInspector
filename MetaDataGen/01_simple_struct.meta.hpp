
#pragma once
#include <cstddef>
#include <type_traits>
#include <tuple>

struct MemberInfo {
    const char* name;
    size_t      offset;
    size_t      size;
    size_t      align;
};

constexpr std::array<MemberInfo, 4> SimpleStruct_meta = {
    { "value", 0, 0, 0 },    { "price", 8, 0, 0 },    { "is_active", 16, 0, 0 },    { "category", 17, 0, 0 }};
