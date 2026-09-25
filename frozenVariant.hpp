#pragma once

#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <cstdio>
#include <new>
#include <type_traits>
#include <utility>
#include <tuple>

/*
 * frozenVariant: a light-weight alternative to std::variant for MCUs.
 *
 * The selected alternative is fixed when the object is first constructed.
 * Storage is dynamically allocated so each instance only uses the memory
 * required for its concrete type (instead of the maximum of all
 * alternatives).  Only a single pointer is stored inside the object; the
 * allocated node carries its type id followed by the concrete value.
 * Objects sharing the same template parameters expose the same API and can
 * be stored together in containers.  The underlying value remains mutable as
 * long as the stored type matches the assignment.
 */
template <typename... Ts>
struct frozenVariant_size_enforcer;

template <typename... Ts>
class frozenVariant {
  static constexpr std::size_t invalid_type = static_cast<std::size_t>(-1);

  template <typename T, typename... List>
  struct index_in_pack;

  template <typename T, typename First, typename... Rest>
  struct index_in_pack<T, First, Rest...> {
    static constexpr std::size_t value =
        std::is_same_v<T, First> ? 0 : 1 + index_in_pack<T, Rest...>::value;
  };

  template <typename T>
  struct index_in_pack<T> {
    static constexpr std::size_t value = invalid_type;
  };

  template <typename T>
  static consteval bool is_supported_type() {
    return (std::is_same_v<T, Ts> || ...);
  }

  template <typename T>
  static consteval std::size_t type_id_for() {
    static_assert(is_supported_type<T>(), "Type not supported by frozenVariant");
    return index_in_pack<T, Ts...>::value;
  }

  struct NodeHeader {
    std::size_t type_id;
  };

  template <typename T>
  struct Node : NodeHeader {
    T value;
  };

  template <typename Fn>
  static decltype(auto) dispatch_by_id(std::size_t id, Fn&& fn) {
    return dispatch_by_id_impl<Fn, Ts...>(id, std::forward<Fn>(fn));
  }

  template <typename Fn, typename First, typename... Rest>
  static decltype(auto) dispatch_by_id_impl(std::size_t id, Fn&& fn) {
    if (id == type_id_for<First>()) {
      return std::forward<Fn>(fn)
          .template operator()<First>();  // expects a templated call operator
    }

    if constexpr (sizeof...(Rest) > 0) {
      return dispatch_by_id_impl<Fn, Rest...>(id, std::forward<Fn>(fn));
    }

    std::abort();
  }

  public:
  frozenVariant() = default;

  template <typename T, typename... Args>
  frozenVariant(std::in_place_type_t<T>, Args&&... args) {
    construct<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  frozenVariant(T&& value)
      : frozenVariant(std::in_place_type_t<std::decay_t<T>>{},
                      std::forward<T>(value)) {}

  frozenVariant(const frozenVariant& other) {
    clone_from(other);
  }

  frozenVariant(frozenVariant&& other) noexcept { move_from(std::move(other)); }

  ~frozenVariant() { reset(); }

  frozenVariant& operator=(const frozenVariant& other) {
    if (this == &other) {
      return *this;
    }

    if (!other.has_value()) {
      reset();
      return *this;
    }

    if (!has_value()) {
      clone_from(other);
      return *this;
    }

    // The stored type cannot change after the first construction.
    if (type_id() != other.type_id()) {
      std::abort();
    }

    dispatch_by_id(type_id(),
                   [&]<typename T>() { get<T>() = other.get<T>(); });

    return *this;
  }

  frozenVariant& operator=(frozenVariant&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    if (!has_value()) {
      move_from(std::move(other));
      return *this;
    }

    if (!other.has_value()) {
      reset();
      return *this;
    }

    if (type_id() != other.type_id()) {
      std::abort();
    }

    dispatch_by_id(type_id(), [&]<typename T>() {
      get<T>() = std::move(other.get<T>());
    });

    other.reset();

    return *this;
  }

  bool has_value() const { return object_ != nullptr; }

  std::size_t type_id() const {
    return object_ ? static_cast<NodeHeader*>(object_)->type_id : invalid_type;
  }

  template <typename T>
  bool holds_alternative() const {
    return has_value() && type_id() == type_id_for<T>();
  }

  template <typename T>
  T& get() {
    using Type = std::decay_t<T>;
    static_assert(is_supported_type<Type>(), "Type not supported by frozenVariant");
    assert(holds_alternative<Type>() && "Accessing frozenVariant with wrong type");
    return static_cast<Node<Type>*>(object_)->value;
  }

  template <typename T>
  const T& get() const {
    using Type = std::decay_t<T>;
    static_assert(is_supported_type<Type>(), "Type not supported by frozenVariant");
    assert(holds_alternative<Type>() && "Accessing frozenVariant with wrong type");
    return static_cast<const Node<Type>*>(object_)->value;
  }

  template <std::size_t I>
  auto& get() {
    static_assert(I < sizeof...(Ts), "Index out of range for frozenVariant");
    using T = std::tuple_element_t<I, std::tuple<Ts...>>;
    return get<T>();
  }

  template <std::size_t I>
  const auto& get() const {
    static_assert(I < sizeof...(Ts), "Index out of range for frozenVariant");
    using T = std::tuple_element_t<I, std::tuple<Ts...>>;
    return get<T>();
  }

  template <typename T>
  void assign(T&& value) {
    static_assert(is_supported_type<std::decay_t<T>>(),
                  "Assigning a type that is not part of frozenVariant");

    if (!holds_alternative<std::decay_t<T>>()) {
      std::abort();
    }

    get<std::decay_t<T>>() = std::forward<T>(value);
  }

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) {
    return visit_impl<Visitor, Ts...>(*this, std::forward<Visitor>(visitor));
  }

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) const {
    return visit_impl<Visitor, Ts...>(*this, std::forward<Visitor>(visitor));
  }

  void reset() {
    if (object_) {
      destroy_node();
      object_ = nullptr;
    }
  }

 private:
  template <typename T, typename... Args>
  void construct(Args&&... args) {
    static_assert(is_supported_type<T>(), "Type not supported by frozenVariant");
    if (object_ != nullptr) {
      return;  // already constructed
    }

    object_ = static_cast<void*>(new Node<T>{
        type_id_for<T>(), T(std::forward<Args>(args)...)});
    if (object_ == nullptr) {
      std::abort();
    }
  }

  void clone_from(const frozenVariant& other) {
    if (!other.has_value()) {
      object_ = nullptr;
      return;
    }

    dispatch_by_id(other.type_id(), [&]<typename T>() {
      const auto* other_node = static_cast<const Node<T>*>(other.object_);
      object_ = static_cast<void*>(
          new Node<T>{other_node->type_id, other_node->value});
      if (object_ == nullptr) {
        std::abort();
      }
    });
  }

  void move_from(frozenVariant&& other) {
    object_ = other.object_;
    other.object_ = nullptr;
  }

  void destroy_node() {
    auto destroy = [&]<typename T>() {
      delete static_cast<Node<T>*>(object_);
    };

    dispatch_by_id(type_id(), destroy);
  }

  template <typename Visitor, typename First, typename... Rest>
  static decltype(auto) visit_impl(frozenVariant& self, Visitor&& visitor) {
    if (self.template holds_alternative<First>()) {
      return std::forward<Visitor>(visitor)(self.template get<First>());
    }

    if constexpr (sizeof...(Rest) > 0) {
      return visit_impl<Visitor, Rest...>(self, std::forward<Visitor>(visitor));
    } else {
      // No stored value: undefined, but avoid undefined behavior here.
      std::abort();
    }
  }

  template <typename Visitor, typename First, typename... Rest>
  static decltype(auto) visit_impl(const frozenVariant& self,
                                   Visitor&& visitor) {
    if (self.template holds_alternative<First>()) {
      return std::forward<Visitor>(visitor)(self.template get<First>());
    }

    if constexpr (sizeof...(Rest) > 0) {
      return visit_impl<Visitor, Rest...>(self, std::forward<Visitor>(visitor));
    } else {
      std::abort();
    }
  }

  void* object_ = nullptr;
  using size_enforcer = frozenVariant_size_enforcer<Ts...>;
};

template <typename T, typename... Ts>
T& get(frozenVariant<Ts...>& variant) {
  using Decayed = std::decay_t<T>;
  static_assert((std::is_same_v<Decayed, Ts> || ...),
                "Type not supported by frozenVariant");
  return variant.template get<Decayed>();
}

template <typename T, typename... Ts>
const T& get(const frozenVariant<Ts...>& variant) {
  using Decayed = std::decay_t<T>;
  static_assert((std::is_same_v<Decayed, Ts> || ...),
                "Type not supported by frozenVariant");
  return variant.template get<Decayed>();
}

template <typename T, typename... Ts>
T&& get(frozenVariant<Ts...>&& variant) {
  using Decayed = std::decay_t<T>;
  static_assert((std::is_same_v<Decayed, Ts> || ...),
                "Type not supported by frozenVariant");
  return std::move(variant.template get<Decayed>());
}

template <std::size_t I, typename... Ts>
auto& get(frozenVariant<Ts...>& variant) {
  static_assert(I < sizeof...(Ts), "Index out of range for frozenVariant");
  using T = std::tuple_element_t<I, std::tuple<Ts...>>;
  return variant.template get<T>();
}

template <std::size_t I, typename... Ts>
const auto& get(const frozenVariant<Ts...>& variant) {
  static_assert(I < sizeof...(Ts), "Index out of range for frozenVariant");
  using T = std::tuple_element_t<I, std::tuple<Ts...>>;
  return variant.template get<T>();
}

template <std::size_t I, typename... Ts>
auto&& get(frozenVariant<Ts...>&& variant) {
  static_assert(I < sizeof...(Ts), "Index out of range for frozenVariant");
  using T = std::tuple_element_t<I, std::tuple<Ts...>>;
  return std::move(variant.template get<T>());
}

template <typename... Ts>
struct frozenVariant_size_enforcer {
  static_assert(sizeof(frozenVariant<Ts...>) == sizeof(void*),
                "frozenVariant stores only a single pointer to its payload");
};

/*
int main()
{
  const char *constant = "constant";
  frozenVariant<int,float, const char*> fv[3]{4, 2.5f, "toto"};
  std::printf("sizeof(fv) = %zu\n", sizeof(fv[0]));

  fv[1] = 3.14f;
  printf("fv[0] = %d fv[1] = %f, fv[2]=%s\n",
         fv[0].get<0>(),
         fv[1].get<1>(),
         fv[2].get<2>());
}
*/
