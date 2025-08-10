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
    { "value", 0, 4, 4 },
    { "price", 8, 8, 8 },
    { "is_active", 16, 1, 1 },
    { "category", 17, 1, 1 }
}}};

