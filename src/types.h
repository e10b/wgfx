#pragma once

#include <cstdint>

#include <glm/glm.hpp>

// Integral types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Network types
using port_t = u16;
using peer_id_t = u8;
using command_t = u16;

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using mat4 = glm::mat4;
using mat3 = glm::mat3;