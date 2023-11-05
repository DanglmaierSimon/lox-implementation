
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include "value.h"

#include <fmt/format.h>
#include <fmt/printf.h>

template<class>
inline constexpr bool always_false_v = false;

std::string toString(const Value& value)
{
  using namespace std;

  auto visitor = [](auto&& arg) -> string {
    using T = decay_t<decltype(arg)>;
    if constexpr (is_same_v<T, double>) {
      return fmt::sprintf("%g", arg);
    } else if constexpr (is_same_v<T, bool>) {
      return arg ? std::string {"true"} : std::string {"false"};
    } else if constexpr (is_same_v<T, monostate>) {
      return std::string {"nil"};
    } else if constexpr (is_same_v<T, Obj*>) {
      return AS_OBJ(arg)->toString();
    } else {
      static_assert(always_false_v<T>,
                    "Non exhaustive visitor for Value variant!");
    }
  };

  return std::visit(visitor, value);
}
