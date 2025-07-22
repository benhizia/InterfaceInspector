
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

constexpr std::array<MemberInfo, 6> InlineCommentTest_meta = {
    { "value", 0, 0, 0 },    { "price", 8, 0, 0 },    { "flag", 16, 0, 0 },    { "type", 17, 0, 0 },    { "ratio", 20, 0, 0 },    { "counter", 24, 0, 0 }};
constexpr std::array<MemberInfo, 4> __NSConstantString_tag_meta = {
    { "isa", 0, 0, 0 },    { "flags", 8, 0, 0 },    { "str", 16, 0, 0 },    { "length", 24, 0, 0 }};
