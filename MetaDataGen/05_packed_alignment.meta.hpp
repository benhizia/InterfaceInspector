
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

constexpr std::array<MemberInfo, 3> __darwin_pthread_handler_rec_meta = {
    { "__routine", 0, 0, 0 },    { "__arg", 8, 0, 0 },    { "__next", 16, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_attr_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_cond_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_condattr_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_mutex_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_mutexattr_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_once_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_rwlock_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 2> _opaque_pthread_rwlockattr_t_meta = {
    { "__sig", 0, 0, 0 },    { "__opaque", 8, 0, 0 }};
constexpr std::array<MemberInfo, 3> _opaque_pthread_t_meta = {
    { "__sig", 0, 0, 0 },    { "__cleanup_stack", 8, 0, 0 },    { "__opaque", 16, 0, 0 }};
constexpr std::array<MemberInfo, 4> AlignedStruct_meta = {
    { "large_value", 0, 0, 0 },    { "precision_value", 8, 0, 0 },    { "small_value", 12, 0, 0 },    { "flag", 14, 0, 0 }};
constexpr std::array<MemberInfo, 5> PackedStruct_meta = {
    { "type_id", 0, 0, 0 },    { "data_value", 1, 0, 0 },    { "checksum", 5, 0, 0 },    { "flags", 7, 0, 0 },    { "timestamp", 8, 0, 0 }};
constexpr std::array<MemberInfo, 7> MixedAlignment_meta = {
    { "normal_int", 0, 0, 0 },    { "aligned_double", 32, 0, 0 },    { "buffer", 40, 0, 0 },    { "flag1", 56, 0, 0 },    { "flag2", 56, 0, 0 },    { "reserved", 56, 0, 0 },    { "value", 57, 0, 0 }};
constexpr std::array<MemberInfo, 4> __NSConstantString_tag_meta = {
    { "isa", 0, 0, 0 },    { "flags", 8, 0, 0 },    { "str", 16, 0, 0 },    { "length", 24, 0, 0 }};
constexpr std::array<MemberInfo, 2> _meta = {
    { "low", 0, 0, 0 },    { "high", 4, 0, 0 }};
