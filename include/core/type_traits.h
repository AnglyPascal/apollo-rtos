#pragma once

#include <type_traits>
#include <utility>

template <typename T>
using remove_ref_cv_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, typename... U>
concept is_any_of = (std::same_as<T, U> || ...);

