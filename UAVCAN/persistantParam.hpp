#pragma once

/**
 * @file persistantParam.hpp
 * @brief Flat in-RAM parameter storage built from constexpr metadata.
 *
 * The UAVCAN parameter defaults live in the compile-time `params_list` frozen map.
 * This header computes, at compile time:
 *  - the ValueKind for each parameter (int/float/bool/string/none),
 *  - the per-parameter size and offset inside a contiguous byte buffer,
 *  - the total buffer size and alignment.
 *
 * At runtime, all parameter values are stored in a single `ramStore` buffer.
 * The `StoredValue` proxy exposes a variant-like API (`visit`, `get`, `holds_alternative`)
 * while reading/writing directly into the flat buffer. This keeps the external API stable
 * (callers still use `Parameter::get/set/find`, PARAM_* macros, std::visit) but removes
 * the per-parameter std::variant footprint in RAM.
 *
 * Usage:
 *  - `Parameter::populateDefaults()` constructs in-place values into `ramStore`.
 *  - `Parameter::find()` returns the StoredValue proxy and the ParamDefault metadata.
 *  - `Parameter::set/get` mirror the previous API; string values live in-place.
 *  - Serialization uses the same on-wire format as before.
 */

#include "dronecan_msgs.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>
#include <etl/vector.h>
#include <span>
#include "persistantParamTypes.hpp"
#include "persistantParamCrc.hpp"
#include "ch.h"
#include "etl/span.h"

namespace Persistant {
  /// Compact in-place string; storage is provided by the flat buffer, not by a pool.
  using StoredString = TinyString<tinyStrSize, tinyStrSize>;
  using StoreSerializeBuffer = etl::vector<uint8_t, 256>;

  /**
   * @brief Runtime kind of a parameter value stored in the flat buffer.
   */
  enum class ValueKind : std::uint8_t {
    None = 0,
    Int  = 1,
    Real = 2,
    Bool = 3,
    Str  = 4,
  };

  /**
   * @brief Per-parameter layout info: byte offset into the flat buffer.
   * @details Size and kind are derived on the fly from `params_list` defaults;
   *          only the offset needs to be stored.
   */
  struct ParamLayout {
    uint16_t offset{};
  };

  /**
   * @brief Full layout of the flat buffer.
   */
  struct LayoutInfo {
    std::array<ParamLayout, params_list_len> entries{};
    uint16_t totalSize{};
    uint16_t maxAlign{};
  };

  /**
   * @brief Deduce the storage kind from the default value.
   */
  inline constexpr ValueKind defaultKind(const FrozenDefault &deflt) {
    return deflt.visit([](const auto &value) {
      using T = std::decay_t<decltype(value)>;
      if constexpr (std::is_same_v<T, NoValue>) {
        return ValueKind::None;
      } else if constexpr (std::is_same_v<T, Integer>) {
        return ValueKind::Int;
      } else if constexpr (std::is_same_v<T, float>) {
        return ValueKind::Real;
      } else if constexpr (std::is_same_v<T, bool>) {
        return ValueKind::Bool;
      } else if constexpr (std::is_same_v<T, frozen::string>) {
        return ValueKind::Str;
      } else {
        static_assert([] { return false; }(), "Unsupported default kind");
      }
    });
  }

  /**
   * @brief Return the byte footprint of a given ValueKind.
   */
  inline constexpr size_t kindSize(ValueKind kind) {
    switch (kind) {
    case ValueKind::None:
      return 0;
    case ValueKind::Int:
      return sizeof(Integer);
    case ValueKind::Real:
      return sizeof(float);
    case ValueKind::Bool:
      return sizeof(bool);
    case ValueKind::Str:
      return sizeof(StoredString);
    }
    return 0;
  }

  /**
   * @brief Return the alignment requirement for a given ValueKind.
   */
  inline constexpr size_t kindAlign(ValueKind kind) {
    switch (kind) {
    case ValueKind::None:
      return 1;
    case ValueKind::Int:
      return alignof(Integer);
    case ValueKind::Real:
      return alignof(float);
    case ValueKind::Bool:
      return alignof(bool);
    case ValueKind::Str:
      return alignof(StoredString);
    }
    return 1;
  }

  /**
   * @brief Compute offset/size/kind for every parameter at compile time.
   * @return LayoutInfo containing per-entry layout and total buffer size.
   */
  inline constexpr LayoutInfo computeLayout() {
    LayoutInfo info{};
    uint16_t offset = 0;
    uint16_t maxAlign = 1;
    for (size_t i = 0; i < params_list_len; i++) {
      const ValueKind kind = defaultKind(params_list[i].second.v);
      const uint16_t align = static_cast<uint16_t>(kindAlign(kind));
      const uint16_t size = static_cast<uint16_t>(kindSize(kind));
      offset = (offset + (align - 1)) & ~(align - 1);
      info.entries[i] = ParamLayout{static_cast<uint16_t>(offset)};
      offset += size;
      maxAlign = std::max<uint16_t>(maxAlign, align);
    }
    info.totalSize = static_cast<uint16_t>(offset);
    info.maxAlign = maxAlign;
    return info;
  }

  template <typename T> struct TypeToKind;
  template <> struct TypeToKind<NoValue> { static constexpr ValueKind value = ValueKind::None; };
  template <> struct TypeToKind<Integer> { static constexpr ValueKind value = ValueKind::Int; };
  template <> struct TypeToKind<float> { static constexpr ValueKind value = ValueKind::Real; };
  template <> struct TypeToKind<bool> { static constexpr ValueKind value = ValueKind::Bool; };
  template <> struct TypeToKind<StoredString> { static constexpr ValueKind value = ValueKind::Str; };

  template <size_t TAG> struct TagToType;
  template <> struct TagToType<0> { using type = NoValue; };
  template <> struct TagToType<1> { using type = Integer; };
  template <> struct TagToType<2> { using type = float; };
  template <> struct TagToType<3> { using type = bool; };
  template <> struct TagToType<4> { using type = StoredString; };

  /**
   * @brief Ensure a ParamDefault's min/max match the default type.
   * @return true when the default and constraints are coherent.
   */
  inline consteval bool isValidDefault(const ParamDefault &param) {
    return param.v.visit(
                      [&](const auto &v) -> bool {
                        using T = std::decay_t<decltype(v)>;

                        if constexpr (std::is_same_v<T, Integer> || std::is_same_v<T, float>) {
                          return std::visit(
                                            [&](const auto &min_val, const auto &max_val) -> bool {
                                              using MinT = std::decay_t<decltype(min_val)>;
                                              using MaxT = std::decay_t<decltype(max_val)>;

                                              if constexpr (std::is_same_v<MinT, T>) {
                                                if (v < min_val)
                                                  return 0;
                                              }
                                              if constexpr (std::is_same_v<MaxT, T>) {
                                                if (v > max_val)
                                                  return 0;
                                              }
                                              return (std::is_same_v<MinT, NoValue> ||
                                                      std::is_same_v<MinT, T>) &&
                                                (std::is_same_v<MaxT, NoValue> ||
                                                 std::is_same_v<MaxT, T>);
                                            },
                                            param.min, param.max);

                        } else {
                          return std::visit(
                                            [](const auto &min_val, const auto &max_val) -> bool {
                                              using MinT = std::decay_t<decltype(min_val)>;
                                              using MaxT = std::decay_t<decltype(max_val)>;

                                              return (std::is_same_v<MinT, NoValue>) &&
                                                (std::is_same_v<MaxT, NoValue>);
                                            },
                                            param.min, param.max);
                        }
                      });
  }

  /**
   * @brief Validate the entire params_list at compile time.
   * @return -1 when all entries are valid, otherwise the failing index.
   */
  inline consteval int validateDefaultsList(const auto &paramsPairList) {
    int index = 0;
    for (const auto &entry : paramsPairList) {
      if (!isValidDefault(entry.second))
        return index;
      index++;
    }
    return -1;
  }

  static_assert(validateDefaultsList(params_list) < 0,
                "❌ params_list contains invalid ParamDefault entries!");

  inline constexpr const auto frozenParameters = frozen::make_map(params_list);
  inline constexpr auto layoutInfo = computeLayout();

  /**
   * @brief Proxy exposing a variant-like API backed by the flat buffer.
   *
   * StoredValue holds only a parameter index. Accessors compute the address in
   * `ramStore` via `layoutInfo` and return references to the in-place objects.
   * This lets existing code keep using std::visit/std::get/std::holds_alternative
   * while we avoid per-parameter std::variant storage.
   */
  class StoredValue;
  class Parameter;

  class StoredValue {
  public:
    /**
     * @brief Create a proxy pointing to a parameter slot.
     * @param idx Index in `params_list` / `layoutInfo.entries` (0-based). The default
     *            constructor leaves the proxy bound to index 0, which is safe because
     *            callers immediately replace it via Parameter::find().
     */
    constexpr StoredValue(size_t idx = 0) : paramIndex(idx) {}

    /**
     * @brief Runtime kind of the referenced parameter.
     * @return ValueKind enumeration describing the active payload type.
     */
    constexpr ValueKind kind() const;

    /**
     * @brief Type tag usable with TagToType.
     * @return Kind encoded as a small integer; matches the variant index.
     */
    constexpr size_t index() const { return static_cast<size_t>(kind()); }

    /**
     * @brief Check if the payload can be viewed as T.
     * @tparam T One of Integer, float, bool, StoredString, NoValue.
     */
    template <typename T>
    constexpr bool is() const;

    /**
     * @brief Mutable typed view of the payload.
     * @tparam T One of the supported parameter types.
     * @return Reference to the object living inside the flat buffer.
     * @warning Asserts if the kind does not match T.
     */
    template <typename T>
    constexpr T &get();

    /**
     * @brief Const typed view of the payload.
     * @tparam T One of the supported parameter types.
     * @return Const reference to the object living inside the flat buffer.
     * @warning Asserts if the kind does not match T.
     */
    template <typename T>
    constexpr const T &get() const;

    /**
     * @brief Access by numeric tag (variant-style).
     * @tparam TAG 0..4 corresponding to NoValue/Integer/float/bool/StoredString.
     */
    template <size_t TAG>
    constexpr auto &get();

    /**
     * @brief Const access by numeric tag (variant-style).
     * @tparam TAG 0..4 corresponding to NoValue/Integer/float/bool/StoredString.
     */
    template <size_t TAG>
    constexpr const auto &get() const;

    /**
     * @brief Assign a new value into the buffer.
     */
    template <typename T>
    constexpr void set(const T &value) { get<T>() = value; }

    /**
     * @brief Apply a visitor over the active payload.
     * @param vis Callable with operator() overloads for each supported type.
     * @return Whatever the visitor returns.
     */
    template <typename Visitor>
    constexpr decltype(auto) visit(Visitor &&vis);

    /**
     * @brief Const-qualified visitor application.
     * @param vis Callable with operator() overloads for each supported type.
     * @return Whatever the visitor returns.
     */
    template <typename Visitor>
    constexpr decltype(auto) visit(Visitor &&vis) const;

  private:
    friend class Parameter;
    constexpr std::byte *raw();
    constexpr const std::byte *raw() const;

    size_t paramIndex{};
  };

  using StoreData = std::pair<ssize_t, StoredValue>;

  /**
   * @brief Facade to access parameters by name/index, get/set, and serialize.
   *
   * The class preserves the previous public surface but now operates on a flat buffer.
   * Callers primarily use:
   *  - find()/cfind()/findIndex() to locate a parameter,
   *  - get()/set() to read or mutate,
   *  - serializeStoredValue()/deserializeStoredValue() for EEPROM/storage.
   *
   * @note All indices are the order of entries in `params_list`.
   */
  class Parameter {
  public:
    Parameter() = delete;

    /**
     * @brief Locate a parameter by its frozen key.
     * @param key `frozen::string` from the table or from a literal.
     * @return 0-based index in `params_list`, or -1 when not found.
     */
    constexpr static ssize_t findIndex(const frozen::string &key);

    /** @brief constexpr-friendly variant of findIndex for compile-time usage. */
    consteval static ssize_t cfindIndex(const frozen::string &key) {
      return findIndex(key);
    }

    /** @overload */
    constexpr static ssize_t findIndex(const uint8_t *str) {
      return findIndex(frozen::string(reinterpret_cast<const char *>(str)));
    }

    /** @overload */
    constexpr static ssize_t findIndex(const char *str) {
      return findIndex(frozen::string(str));
    }

    /**
     * @brief Get the parameter name by index (no bounds check).
     * @param index 0-based index within `params_list`.
     */
    constexpr static const frozen::string &findName(const size_t index);

    /**
     * @brief Retrieve both the StoredValue proxy and its metadata.
     * @param index 0-based index; out-of-range returns a dummy NoValue pair.
     */
    constexpr static std::pair<StoredValue, const ParamDefault &>
    find(const ssize_t index);

    /** @overload Retrieve by frozen key. */
    constexpr static std::pair<StoredValue, const ParamDefault &>
    find(const frozen::string key);

    /** @brief constexpr wrapper over find() for compile-time literals. */
    consteval static std::pair<StoredValue, const ParamDefault &>
    cfind(const frozen::string key);

    /** @overload Retrieve by C string key. */
    constexpr static std::pair<StoredValue, const ParamDefault &>
    find(const char *key);

    /**
     * @brief Typed accessor by name; asserts on mismatched type.
     * @tparam T Desired runtime type (Integer/float/bool/StoredString).
     * @param key Parameter name.
     * @return Const reference to the in-place value.
     */
    template <typename T>
    static const T &get(const char *key);

    /** @overload Typed accessor by index. */
    template <typename T>
    static const T &get(const size_t index);

    /**
     * @brief Variant-style getter by tag.
     * @tparam TAG 0..4 corresponding to TagToType entries.
     * @param index Parameter index.
     */
    template <size_t TAG>
    static const auto &get(const size_t index);

    /**
     * @brief Clamp an Integer against its ParamDefault min/max.
     */
    constexpr static Integer clamp(const ParamDefault &deflt,
                                   const Integer &value);

    /**
     * @brief Clamp a float against its ParamDefault min/max.
     */
    constexpr static float clamp(const ParamDefault &deflt, const float &value);

    /**
     * @brief Assign an Integer value (applies clamping).
     * @return true if the storage kind matches.
     */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const Integer &value);

    /** @overload Assign a bool. */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const bool &value);

    /** @overload Assign a float (applies clamping). */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const float &value);

    /** @overload Assign a TinyString (copy into flat buffer slot). */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const StoredString &value);

    /** @overload Parse and assign from a textual literal. */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const char *value);

    /** @overload Assign from the frozen default variant. */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const FrozenDefault &value);

    /** @overload Assign from another StoredValue (same kind only). */
    constexpr static bool
    set(const std::pair<StoredValue, const ParamDefault &> &p,
        const StoredValue &value);

    /**
     * @brief Copy out the current value with compile-time type.
     * @tparam T Desired return type.
     */
    template <typename T>
    constexpr static T
    get(const std::pair<StoredValue, const ParamDefault &> &p);

    /**
     * @brief Serialize a parameter into a transient buffer for EEPROM.
     * @param index Parameter index.
     * @param buffer Output buffer reused by storage.
     */
    static void serializeStoredValue(size_t index, StoreSerializeBuffer &buffer);

    /**
     * @brief Deserialize a parameter from a buffer back into ramStore.
     * @param index Parameter index to overwrite.
     * @param buffer Input buffer filled by storage backend.
     * @return true when type and payload are consistent; false otherwise.
     */
    static bool deserializeStoredValue(size_t index, const StoreSerializeBuffer &buffer);

    /**
     * @brief Extract the parameter name slice from a serialized buffer.
     * @return Span pointing to the null-terminated name (first byte holds length).
     */
    static etl::span<const uint8_t> deserializeGetName(const StoreSerializeBuffer &buffer);

    /**
     * @brief Construct all default values directly into the flat buffer.
     */
    static void populateDefaults();

    /**
     * @brief Clamp a single parameter against its min/max (no-op for strings/bools).
     */
    static void enforceMinMax(size_t index);

    /**
     * @brief Clamp all parameters; used after deserialization.
     */
    static void enforceMinMax();

    /// Flat contiguous backing buffer holding every parameter value.
    alignas(layoutInfo.maxAlign) static inline std::array<std::byte, layoutInfo.totalSize> ramStore{};
  };

  // StoredValue definitions
  inline constexpr ValueKind StoredValue::kind() const {
    return defaultKind(std::next(frozenParameters.begin(), paramIndex)->second.v);
  }

  inline constexpr std::byte *StoredValue::raw() {
    return Parameter::ramStore.data() + layoutInfo.entries[paramIndex].offset;
  }

  inline constexpr const std::byte *StoredValue::raw() const {
    return Parameter::ramStore.data() + layoutInfo.entries[paramIndex].offset;
  }

  template <typename T>
  inline constexpr bool StoredValue::is() const {
    return kind() == TypeToKind<T>::value;
  }

  template <typename T>
  inline constexpr T &StoredValue::get() {
    chDbgAssert(is<T>(), "invalid tag for storage");
    return *std::launder(reinterpret_cast<T *>(raw()));
  }

  template <typename T>
  inline constexpr const T &StoredValue::get() const {
    chDbgAssert(is<T>(), "invalid tag for storage");
    return *std::launder(reinterpret_cast<const T *>(raw()));
  }

  template <size_t TAG>
  inline constexpr auto &StoredValue::get() {
    using T = typename TagToType<TAG>::type;
    return get<T>();
  }

  template <size_t TAG>
  inline constexpr const auto &StoredValue::get() const {
    using T = typename TagToType<TAG>::type;
    return get<T>();
  }

  template <typename Visitor>
  inline constexpr decltype(auto) StoredValue::visit(Visitor &&vis) {
    switch (kind()) {
    case ValueKind::None:
      return std::forward<Visitor>(vis)(NoValue{});
    case ValueKind::Int:
      return std::forward<Visitor>(vis)(get<Integer>());
    case ValueKind::Real:
      return std::forward<Visitor>(vis)(get<float>());
    case ValueKind::Bool:
      return std::forward<Visitor>(vis)(get<bool>());
    case ValueKind::Str:
      return std::forward<Visitor>(vis)(get<StoredString>());
    }
    return std::forward<Visitor>(vis)(NoValue{});
  }

  template <typename Visitor>
  inline constexpr decltype(auto) StoredValue::visit(Visitor &&vis) const {
    switch (kind()) {
    case ValueKind::None:
      return std::forward<Visitor>(vis)(NoValue{});
    case ValueKind::Int:
      return std::forward<Visitor>(vis)(get<Integer>());
    case ValueKind::Real:
      return std::forward<Visitor>(vis)(get<float>());
    case ValueKind::Bool:
      return std::forward<Visitor>(vis)(get<bool>());
    case ValueKind::Str:
      return std::forward<Visitor>(vis)(get<StoredString>());
    }
    return std::forward<Visitor>(vis)(NoValue{});
  }

  // Parameter inline definitions
  inline constexpr ssize_t Parameter::findIndex(const frozen::string &key) {
    const auto it = frozenParameters.find(key);
    if (it == frozenParameters.end()) {
      return -1;
    }
    return std::distance(frozenParameters.begin(), it);
  }
  
  inline constexpr const frozen::string &Parameter::findName(const size_t index)  {
    return std::next(frozenParameters.begin(), index < params_list_len ? index : 0)->first;
  }

  inline constexpr std::pair<StoredValue, const ParamDefault &>
  Parameter::find(const ssize_t index) {
    static constexpr ParamDefault empty = {};
    if ((index >= 0) and (index < params_list_len)) {
      return {StoredValue(index), std::next(frozenParameters.begin(), index)->second};
    } else {
      return {StoredValue(0), empty};
    }
  }

  inline constexpr std::pair<StoredValue, const ParamDefault &>
  Parameter::find(const frozen::string key) {
    const auto index = findIndex(key);
    return find(index);
  }

  inline consteval std::pair<StoredValue, const ParamDefault &>
  Parameter::cfind(const frozen::string key) {
    const auto index = cfindIndex(key);
    return find(index);
  }

  inline constexpr std::pair<StoredValue, const ParamDefault &>
  Parameter::find(const char *key) {
    const auto index = findIndex(frozen::string(key));
    return find(index);
  }

  template<typename T>
  inline const T& Parameter::get(const char *key)
  {
    const auto& [stored, _] = find(key);
    return stored.get<T>();
  }

  template<typename T>
  inline const T& Parameter::get(const size_t index)
  {
    const auto& [stored, _] = find(index);
    return stored.get<T>();
  }
  
  template<size_t TAG>
  inline const auto& Parameter::get(const size_t index)
  {
    const auto& [stored, _] = find(index);
    return stored.get<TAG>();
  }

  inline constexpr Integer Parameter::clamp(const ParamDefault &deflt,
                                     const Integer &value) {
    Integer ret = value;

    if (std::holds_alternative<Integer>(deflt.min))
      ret = std::max(ret, std::get<Integer>(deflt.min));
    if (std::holds_alternative<Integer>(deflt.max))
      ret = std::min(ret, std::get<Integer>(deflt.max));
    return ret;
  }

  inline constexpr float Parameter::clamp(const ParamDefault &deflt,
                                   const float &value) {
    float ret = value;

    if (std::holds_alternative<float>(deflt.min))
      ret = std::max(ret, std::get<float>(deflt.min));
    if (std::holds_alternative<float>(deflt.max))
      ret = std::min(ret, std::get<float>(deflt.max));
    return ret;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const Integer &value) {
    auto [store, deflt] = p;
    if (store.kind() != ValueKind::Int) {
      return false;
    }
    store.get<Integer>() = clamp(deflt, value);
    return true;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const bool &value) {
    auto [store, deflt] = p;
    (void) deflt;
    if (store.kind() != ValueKind::Bool) {
      return false;
    }
    store.get<bool>() = value;
    return true;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const float &value) {
    auto [store, deflt] = p;
    if (store.kind() != ValueKind::Real) {
      return false;
    }
    store.get<float>() = clamp(deflt, value);
    return true;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const StoredString &value) {
    auto [store, deflt] = p;
    (void) deflt;
    if (store.kind() != ValueKind::Str) {
      return false;
    }
    store.get<StoredString>() = value;
    return true;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const char *value) {
    auto [store, deflt] = p;
    switch (store.kind()) {
    case ValueKind::Int:
      store.get<Integer>() = clamp(deflt, atoll(value));
      break;
    case ValueKind::Bool:
      store.get<bool>() = (*value == '0') or (tolower(*value) == 'f') ? false : true;
      break;
    case ValueKind::Real:
      store.get<float>() = clamp(deflt, static_cast<float>(atof(value)));
      break;
    case ValueKind::Str:
      store.get<StoredString>() = value;
      break;
    default:
      return false;
    }
    return true;
  }

  inline constexpr  bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const FrozenDefault &value)
  {
    auto [store, deflt] = p;
    bool ok = true;
    value.visit([&](const auto& val) {
      using T = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<T, NoValue>) {
        ok = store.kind() == ValueKind::None;
      } else if constexpr (std::is_same_v<T, Integer>) {
        ok = set(p, val);
      } else if constexpr (std::is_same_v<T, float>) {
        ok = set(p, val);
      } else if constexpr (std::is_same_v<T, bool>) {
        ok = set(p, val);
      } else if constexpr (std::is_same_v<T, frozen::string>) {
        if (store.kind() == ValueKind::Str) {
          store.get<StoredString>() = val.data();
        } else {
          ok = false;
        }
      }
    });
    return ok;
  }

  inline constexpr bool
  Parameter::set(const std::pair<StoredValue, const ParamDefault &> &p,
                 const StoredValue &value)
  {
    auto [store, deflt] = p;
    if (store.kind() != value.kind()) {
      return false;
    }
    switch (store.kind()) {
    case ValueKind::Int:
      store.get<Integer>() = clamp(deflt, value.get<Integer>());
      break;
    case ValueKind::Real:
      store.get<float>() = clamp(deflt, value.get<float>());
      break;
    case ValueKind::Bool:
      store.get<bool>() = value.get<bool>();
      break;
    case ValueKind::Str:
      store.get<StoredString>() = value.get<StoredString>();
      break;
    case ValueKind::None:
      break;
    }
    return true;
  }

  
  template <typename T>
  inline constexpr T
  Parameter::get(const std::pair<StoredValue, const ParamDefault &> &p) {
    const auto &[store, deflt] = p;
    (void) deflt;
    return store.get<T>();
  }

  /**
   * @brief Convert a StoredValue (runtime value) to a UAVCAN wire representation.
   * @param storedValue Flat-buffer proxy.
   * @param uavcanValue Destination UAVCAN union.
   */
  void toUavcan(const StoredValue &storedValue, uavcan_protocol_param_Value &uavcanValue);

  /**
   * @brief Convert a FrozenDefault (compile-time default) to a UAVCAN union.
   * @param defaultValue Variant of default.
   * @param uavcanValue Destination UAVCAN union.
   */
  void toUavcan(const FrozenDefault &defaultValue, uavcan_protocol_param_Value &uavcanValue);

  /**
   * @brief Import a UAVCAN union into a StoredValue if the types match.
   * @return true on success, false on kind mismatch.
   */
  bool fromUavcan(const uavcan_protocol_param_Value &uavcanValue, StoredValue &storedValue);

  /**
   * @brief Convert NumericValue min/max to UAVCAN.
   */
  void toUavcan(const NumericValue &numericValue, uavcan_protocol_param_NumericValue &uavcanValue);

  /**
   * @brief Populate NumericValue from UAVCAN.
   * @return true when the tag is recognized.
   */
  bool fromUavcan(const uavcan_protocol_param_NumericValue &uavcanValue, NumericValue &numericValue);

  /**
   * @brief Handle a UAVCAN GetSet request and optionally mutate storage.
   * @param req Incoming request.
   * @param resp Response filled with current/default/min/max.
   * @return Pair of (index, StoredValue) when a value was modified, {-1, {}} otherwise.
   */
  StoreData getSetResponse(const uavcan_protocol_param_GetSetRequest &req,
                           uavcan_protocol_param_GetSetResponse &resp);

} // namespace Persistant

// Provide std::visit/std::get/std::holds_alternative compatibility so existing
// callers keep working with the lightweight StoredValue proxy.
namespace std {
  template <class Visitor>
  constexpr decltype(auto) visit(Visitor &&vis, Persistant::StoredValue &v) {
    return v.visit(std::forward<Visitor>(vis));
  }

  template <class Visitor>
  constexpr decltype(auto) visit(Visitor &&vis, const Persistant::StoredValue &v) {
    return v.visit(std::forward<Visitor>(vis));
  }

  template <class T>
  constexpr T &get(Persistant::StoredValue &v) {
    return v.template get<T>();
  }

  template <class T>
  constexpr const T &get(const Persistant::StoredValue &v) {
    return v.template get<T>();
  }

  template <class T>
  constexpr bool holds_alternative(const Persistant::StoredValue &v) {
    return v.template is<T>();
  }
} // namespace std

#define PARAM_TYPECGET(type, name)                                     \
  ({                                                                   \
     constexpr ssize_t idx = Persistant::Parameter::findIndex(name);   \
     static_assert(idx >= 0, name " not found");                       \
     Persistant::Parameter::get<type>(idx);                            \
  })

#define PARAM_CGET(name)                                               \
  ({                                                                   \
     constexpr ssize_t idx = Persistant::Parameter::findIndex(name);   \
     static_assert(idx >= 0, name " not found");                       \
     constexpr auto &variant =                                         \
       std::next(Persistant::frozenParameters.begin(), idx)->second.v; \
     constexpr int tag = variant.index();                              \
     Persistant::Parameter::get<tag>(idx);                             \
  })
