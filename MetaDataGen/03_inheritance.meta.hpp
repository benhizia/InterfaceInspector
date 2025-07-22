
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

constexpr std::array<MemberInfo, 4> __NSConstantString_tag_meta = {
    { "isa", 0, 0, 0 },    { "flags", 8, 0, 0 },    { "str", 16, 0, 0 },    { "length", 24, 0, 0 }};
constexpr std::array<MemberInfo, 3> Shape_meta = {
    { "x_", 8, 0, 0 },    { "y_", 16, 0, 0 },    { "color_", 24, 0, 0 }};
constexpr std::array<MemberInfo, 2> Rectangle_meta = {
    { "width_", 32, 0, 0 },    { "height_", 40, 0, 0 }};
