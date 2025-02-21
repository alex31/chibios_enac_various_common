#pragma once

#include "frozen/set.h"
#include "frozen/string.h"
#include "frozen/map.h"
#include "tinyString.hpp"
#include <array>
#include <variant>

namespace Persistant {
/// must be (4*N)-1 to avoid to spill ram in padding
constexpr size_t tinyStrSize = 47;

// Integer type choice : int64_t or int32_t
// if int64_t : mirror DSDL message format, but each param use 16 bytes in ram
// if int32_t : does NOT mirror DSDL message format, but each param use 8 bytes
// in ram
using Integer = int64_t;

/** @brief Represents an empty value in the parameter system. */
struct NoValue {};

using Default = std::variant<NoValue, Integer, float, bool, frozen::string>;
using NumericValue = std::variant<NoValue, Integer, float>;

/**
 * @brief Stores default values and constraints for a parameter.
 */
struct ParamDefault {
  NumericValue min = (NoValue){};
  NumericValue max = (NoValue){};
  Default v = (NoValue){};
};

/// Default parameter list stored in ROM.
using ParamDefaultEntry =  std::pair<frozen::string, ParamDefault>;
static constexpr  ParamDefaultEntry params_list[] {
#include "nodeParameters.hpp"
};

constexpr ssize_t params_list_len =
    sizeof(params_list) / sizeof(params_list[0]);

}
