#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <limits>
#include <type_traits>

namespace UAVCAN {

/**
 * @brief Concept for DSDL strings: an aggregate with `len` and array `data`.
 */
template <typename T>
concept DsdlStringLike =
  requires(T t) {
    { t.len } -> std::convertible_to<std::size_t>;
    requires std::is_array_v<std::remove_reference_t<decltype(t.data)>>;
  };

/**
 * @brief Concept for views providing contiguous data and size.
 */
template <typename View>
concept DsdlViewLike =
  requires(const View &v) {
    { v.data() } -> std::convertible_to<const char *>;
    { v.size() } -> std::convertible_to<std::size_t>;
  };

template <DsdlStringLike String>
constexpr std::size_t dsdlStringCapacity(const String &s)
{
  using Data = std::remove_reference_t<decltype(s.data)>;
  return std::extent_v<Data>;
}

/**
 * @brief Assign from a C-string into a DSDL string buffer. Returns false on truncation.
 */
template <DsdlStringLike String>
bool dsdlAssign(String &dst, const char *src, std::size_t srcLen)
{
  const std::size_t cap = dsdlStringCapacity(dst);
  const std::size_t maxLen =
    std::min<std::size_t>(cap, std::numeric_limits<decltype(dst.len)>::max());
  const std::size_t copyLen = std::min(srcLen, maxLen);

  if (copyLen > 0U) {
    std::memcpy(dst.data, src, copyLen);
  }
  dst.len = static_cast<decltype(dst.len)>(copyLen);
  if (copyLen < cap) {
    dst.data[copyLen] = 0;
  }
  return copyLen == srcLen;
}

template <DsdlStringLike String>
bool dsdlAssign(String &dst, const char *src)
{
  return dsdlAssign(dst, src, std::strlen(src));
}

template <DsdlStringLike String, DsdlViewLike View>
bool dsdlAssign(String &dst, const View &view)
{
  return dsdlAssign(dst, view.data(), view.size());
}

/**
 * @brief Append a C-string to a DSDL string buffer. Returns false on truncation.
 */
template <DsdlStringLike String>
bool dsdlAppend(String &dst, const char *suffix, std::size_t suffixLen)
{
  const std::size_t cap = dsdlStringCapacity(dst);
  const std::size_t baseLen = std::min<std::size_t>(static_cast<std::size_t>(dst.len), cap);
  const std::size_t maxLen =
    std::min<std::size_t>(cap, std::numeric_limits<decltype(dst.len)>::max());
  const std::size_t available = (baseLen >= maxLen) ? 0U : maxLen - baseLen;
  const std::size_t appendLen = std::min(suffixLen, available);

  if (appendLen > 0U) {
    std::memcpy(dst.data + baseLen, suffix, appendLen);
  }
  const std::size_t newLen = baseLen + appendLen;
  if (newLen < cap) {
    dst.data[newLen] = 0;
  }
  dst.len = static_cast<decltype(dst.len)>(newLen);
  return appendLen == suffixLen;
}

template <DsdlStringLike String>
bool dsdlAppend(String &dst, const char *suffix)
{
  return dsdlAppend(dst, suffix, std::strlen(suffix));
}

template <DsdlStringLike String, DsdlViewLike View>
bool dsdlAppend(String &dst, const View &suffix)
{
  return dsdlAppend(dst, suffix.data(), suffix.size());
}

} // namespace UAVCAN
