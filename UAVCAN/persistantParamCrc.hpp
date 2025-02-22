#pragma once


#include "persistantParamTypes.hpp"
using namespace Persistant;

#include <cstdint>
#include <array>
#include <bit>       // for std::bit_cast (C++20)
#include <utility>
#include <variant>
#include <type_traits>



//--------------------------------------------------------------------
// 1) A constexpr CRC lookup table for polynomial 0xEDB88320
//--------------------------------------------------------------------
static constexpr auto make_crc32_table() {
    std::array<std::uint32_t, 256> table = {};
    for (std::uint32_t i = 0; i < 256; i++) {
        std::uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c >> 1) ^ (0xEDB88320u & -(c & 1u));
        }
        table[i] = c;
    }
    return table;
}

static constexpr std::array<std::uint32_t, 256> CRC_TABLE = make_crc32_table();

//--------------------------------------------------------------------
// 2) Bytewise update function
//--------------------------------------------------------------------
constexpr std::uint32_t crc32_update_byte(std::uint32_t crc, std::uint8_t byte)
{
    std::uint8_t idx = static_cast<std::uint8_t>(crc ^ byte);
    return (crc >> 8) ^ CRC_TABLE[idx];
}

//--------------------------------------------------------------------
// 3) Update from a contiguous block of bytes (without reinterpret_cast)
//--------------------------------------------------------------------
constexpr std::uint32_t crc32_update_block(
    std::uint32_t crc,
    const char* data,
    std::size_t length)
{
    for (std::size_t i = 0; i < length; i++) {
        crc = crc32_update_byte(crc, static_cast<std::uint8_t>(data[i]));
    }
    return crc;
}

// Overload for any array of uint8_t if you prefer:
constexpr std::uint32_t crc32_update_block(
    std::uint32_t crc,
    const std::uint8_t* data,
    std::size_t length)
{
    for (std::size_t i = 0; i < length; i++) {
        crc = crc32_update_byte(crc, data[i]);
    }
    return crc;
}

//--------------------------------------------------------------------
// 4) Hashing a POD by turning it into an array of bytes via std::bit_cast
//--------------------------------------------------------------------
template <typename T>
constexpr std::enable_if_t<std::is_standard_layout_v<T>, std::uint32_t>
crc32_hash_pod(std::uint32_t crc, const T& value)
{
#if __cpp_lib_bit_cast >= 201806L  // Check if bit_cast is available
    auto bytes = std::bit_cast<std::array<std::uint8_t, sizeof(T)>>(value);
    for (auto b : bytes) {
        crc = crc32_update_byte(crc, b);
    }
#else
    // Fallback for C++17 (union-based approach shown below)
    // ...
#endif
    return crc;
}

//--------------------------------------------------------------------
// 5) Hashing for your variant types
//--------------------------------------------------------------------
constexpr std::uint32_t hashVariant(std::uint32_t crc, const FrozenDefault& dv);

constexpr std::uint32_t hashVariant(std::uint32_t crc, const NumericValue& nv);

// Hash a single ParamDefault
constexpr std::uint32_t hashParamDefault(std::uint32_t crc, const ParamDefault& pd)
{
    crc = hashVariant(crc, pd.min);
    crc = hashVariant(crc, pd.max);
    crc = hashVariant(crc, pd.v);
    return crc;
}

// Hash a single ParamDefaultEntry (key + ParamDefault)
constexpr std::uint32_t hashParamDefaultEntry(std::uint32_t crc, const ParamDefaultEntry& entry)
{
    // Hash the string's bytes:
    crc = crc32_update_block(crc, entry.first.data(), entry.first.size());
    // Then hash the ParamDefault object
    crc = hashParamDefault(crc, entry.second);
    return crc;
}

//--------------------------------------------------------------------
// 6) Provide the variant visitation logic
//--------------------------------------------------------------------
constexpr std::uint32_t hashVariant(std::uint32_t crc, const FrozenDefault& dv)
{
    return std::visit(
        [&](auto&& arg) -> std::uint32_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NoValue>) {
                // incorporate some tag
                return crc32_hash_pod(crc, 0u);
            } else if constexpr (std::is_same_v<T, Integer>) {
                return crc32_hash_pod(crc, arg);
            } else if constexpr (std::is_same_v<T, float>) {
                return crc32_hash_pod(crc, arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                std::uint8_t b = arg ? 1 : 0;
                return crc32_hash_pod(crc, b);
            } else if constexpr (std::is_same_v<T, frozen::string>) {
                // hash the bytes of the string
                return crc32_update_block(crc, arg.data(), arg.size());
            }
        },
        dv
    );
}

constexpr std::uint32_t hashVariant(std::uint32_t crc, const NumericValue& nv)
{
    return std::visit(
        [&](auto&& arg) -> std::uint32_t {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, NoValue>) {
                // incorporate some tag
                return crc32_hash_pod(crc, 0u);
            } else if constexpr (std::is_same_v<T, Integer>) {
                return crc32_hash_pod(crc, arg);
            } else if constexpr (std::is_same_v<T, float>) {
                return crc32_hash_pod(crc, arg);
            }
        },
        nv
    );
}

//--------------------------------------------------------------------
// 7) The consteval or constexpr function to compute the final CRC
//--------------------------------------------------------------------
// constexpr std::uint32_t computeParamsListCRC()
// {
//     std::uint32_t crc = 0xFFFFFFFFu; // initial
//     for (auto&& entry : params_list) {
//         crc = hashParamDefaultEntry(crc, entry);
//     }
//     return ~crc; // final XOR
// }

// int main()
// {
//    static constexpr std::uint32_t params_list_crc32 = computeParamsListCRC();
//    return params_list_crc32;
// }
