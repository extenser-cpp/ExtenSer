#ifndef CONTAINER_ADAPTERS_SPAN_HPP
#define CONTAINER_ADAPTERS_SPAN_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>

#if defined(__cpp_lib_span)
#  include <span>
#endif

namespace extenser
{
#if defined(__cpp_lib_span)
template<typename T, size_t N>
struct container_adapter<span<T, N>>
#else
template<typename T>
struct container_adapter<span<T>>
#endif
{
#if defined(__cpp_lib_span)
    using container_type = span<T, N>;
#else
    using container_type = span<T>;
#endif

    using value_type = typename container_type::value_type;

#if defined(__cpp_lib_span)
    static constexpr bool has_dynamic_extent = N == std::dynamic_extent;
    static constexpr size_t max_extent = N;
#else
    static constexpr bool has_dynamic_extent = true;
    static constexpr size_t max_extent = (std::numeric_limits<size_t>::max)();
#endif

    static constexpr bool is_mutable = !std::is_const_v<T>;

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        std::transform(first, last, container.begin(), convert_fn);
    }
};
} //namespace extenser
#endif
