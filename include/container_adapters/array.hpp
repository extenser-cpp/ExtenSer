#ifndef CONTAINER_ADAPTERS_ARRAY_HPP
#define CONTAINER_ADAPTERS_ARRAY_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

namespace extenser
{
template<typename T, size_t N>
struct container_adapter<std::array<T, N>>
{
    using container_type = std::array<T, N>;
    using value_type = typename container_type::value_type;

    static constexpr bool has_dynamic_extent = false;
    static constexpr size_t max_extent = N;
    static constexpr bool is_mutable = !std::is_const_v<T>;

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(std::distance(first, last) == max_extent);
        std::transform(first, last, container.begin(), convert_fn);
    }
};
} //namespace extenser
#endif
