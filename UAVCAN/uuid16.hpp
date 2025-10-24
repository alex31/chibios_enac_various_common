#pragma once

/*
 * UAVCAN et diffusion des UUID
 * ----------------------------
 *
 * UAVCAN identifie chaque nœud par un UUID de 128 bits (16 octets).
 * Sur STM32, le microcontrôleur expose seulement un identifiant unique matériel
 * de 96 bits (12 octets) via UID_BASE. Il faut donc transformer cet UUID_12 en UUID_16.
 *
 * Exigence principale : la *diffusion*.
 * Dans le protocole UAVCAN, l’algorithme de dynamic node ID allocation échange
 * des fragments (chunks) de 48 bits (6 octets) extraits du UUID complet.
 * Deux cartes ayant des UUID_12 proches risqueraient de générer des chunks identiques
 * si on se contentait d’un simple padding ou d’une copie directe.
 *
 * Solution retenue :
 * - Hacher `domain || uuid12` avec MurmurHash3_128 (non cryptographique,
 *   mais très léger et avec une excellente avalanche).
 * - Retourner les 16 octets du digest comme UUID_16.
 *
 * Ce mécanisme garantit :
 * - Compatibilité avec UAVCAN (UUID sur 16 octets).
 * - Bonne séparation entre UUID même proches à l’origine.
 * - Empreinte minimale en flash et en pile, adaptée aux STM32 sous ChibiOS.
 */

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>

namespace uuid_map {

constexpr const char kDomain[] = "UAVCAN:STM32-UUID:v1";
constexpr std::uint32_t kSeed  = 0xBADC0FFEu;

static_assert(std::endian::native == std::endian::little,
              "This file assumes a little-endian target (STM32).");

constexpr std::uint32_t fmix32(std::uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return h;
}

// MurmurHash3_128 sur un buffer, sortie h[4] (4 x 32 bits)
inline void murmur3_128(const std::uint8_t* data, std::uint32_t len,
                            std::uint32_t seed, std::uint32_t h[4]) {
    std::uint32_t h1 = seed, h2 = seed, h3 = seed, h4 = seed;
    constexpr std::uint32_t c1 = 0x239b961bU, c2 = 0xab0e9789U;
    constexpr std::uint32_t c3 = 0x38b34ae5U, c4 = 0xa1e38b93U;

    const int nblocks = static_cast<int>(len / 16);
    const std::uint8_t* p = data;

    for (int i = 0; i < nblocks; ++i, p += 16) {
        std::uint32_t k1, k2, k3, k4;
        std::memcpy(&k1, p + 0, 4);
        std::memcpy(&k2, p + 4, 4);
        std::memcpy(&k3, p + 8, 4);
        std::memcpy(&k4, p + 12, 4);

        k1 *= c1; k1 = std::rotl(k1,15); k1 *= c2; h1 ^= k1;
        h1 = std::rotl(h1,19); h1 += h2; h1 = h1*5U + 0x561ccd1bU;

        k2 *= c2; k2 = std::rotl(k2,16); k2 *= c3; h2 ^= k2;
        h2 = std::rotl(h2,17); h2 += h3; h2 = h2*5U + 0x0bcaa747U;

        k3 *= c3; k3 = std::rotl(k3,17); k3 *= c4; h3 ^= k3;
        h3 = std::rotl(h3,15); h3 += h4; h3 = h3*5U + 0x96cd1c35U;

        k4 *= c4; k4 = std::rotl(k4,18); k4 *= c1; h4 ^= k4;
        h4 = std::rotl(h4,13); h4 += h1; h4 = h4*5U + 0x32ac3b17U;
    }

    // Tail
    const std::uint8_t* tail = data + (nblocks * 16);
    const int rem = len & 15;

    std::uint32_t k1 = 0;
    std::uint32_t k2 = 0;
    std::uint32_t k3 = 0;
    std::uint32_t k4 = 0;

    switch(rem)
    {
    case 15: k4 ^= static_cast<std::uint32_t>(tail[14]) << 16; [[fallthrough]];
    case 14: k4 ^= static_cast<std::uint32_t>(tail[13]) << 8;  [[fallthrough]];
    case 13: k4 ^= static_cast<std::uint32_t>(tail[12]) << 0;
             k4 *= c4; k4 = std::rotl(k4,18); k4 *= c1; h4 ^= k4; [[fallthrough]];
    case 12: k3 ^= static_cast<std::uint32_t>(tail[11]) << 24; [[fallthrough]];
    case 11: k3 ^= static_cast<std::uint32_t>(tail[10]) << 16; [[fallthrough]];
    case 10: k3 ^= static_cast<std::uint32_t>(tail[ 9]) << 8;  [[fallthrough]];
    case  9: k3 ^= static_cast<std::uint32_t>(tail[ 8]) << 0;
             k3 *= c3; k3 = std::rotl(k3,17); k3 *= c4; h3 ^= k3; [[fallthrough]];
    case  8: k2 ^= static_cast<std::uint32_t>(tail[ 7]) << 24; [[fallthrough]];
    case  7: k2 ^= static_cast<std::uint32_t>(tail[ 6]) << 16; [[fallthrough]];
    case  6: k2 ^= static_cast<std::uint32_t>(tail[ 5]) << 8;  [[fallthrough]];
    case  5: k2 ^= static_cast<std::uint32_t>(tail[ 4]) << 0;
             k2 *= c2; k2 = std::rotl(k2,16); k2 *= c3; h2 ^= k2; [[fallthrough]];
    case  4: k1 ^= static_cast<std::uint32_t>(tail[ 3]) << 24; [[fallthrough]];
    case  3: k1 ^= static_cast<std::uint32_t>(tail[ 2]) << 16; [[fallthrough]];
    case  2: k1 ^= static_cast<std::uint32_t>(tail[ 1]) << 8;  [[fallthrough]];
    case  1: k1 ^= static_cast<std::uint32_t>(tail[ 0]) << 0;
             k1 *= c1; k1 = std::rotl(k1,15); k1 *= c2; h1 ^= k1;
    }

    // Finalisation
    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix32(h1); h2 = fmix32(h2); h3 = fmix32(h3); h4 = fmix32(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h[0] = h1; h[1] = h2; h[2] = h3; h[3] = h4;
}

// API principale : entrée pointeur 12 octets (UID_BASE), sortie std::array<16>
inline std::array<std::uint8_t,16>
uuid16_from_uuid12(const std::uint8_t* uuid12) {
    constexpr std::size_t dom_len = sizeof(kDomain) - 1;
    std::array<std::uint8_t, dom_len + 12> msg{};
    std::memcpy(msg.data(), kDomain, dom_len);
    std::memcpy(msg.data() + dom_len, uuid12, 12);

    std::uint32_t h[4];
    murmur3_128(msg.data(), static_cast<std::uint32_t>(msg.size()), kSeed, h);

    std::array<std::uint8_t,16> out{};
    std::memcpy(out.data(), h, 16);  // LE natif, direct
    return out;
}

} // namespace uuid_map
