#ifndef CONTAINER_ADAPTERS_FORWARD_LIST_HPP
#define CONTAINER_ADAPTERS_FORWARD_LIST_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <iterator>
#include <limits>

namespace extenser
{
template<typename T, typename Allocator>
struct container_adapter<std::forward_list<T, Allocator>>
{
    using container_type = std::forward_list<T, Allocator>;
    using value_type = typename container_type::value_type;

    static constexpr bool has_dynamic_extent = true;
    static constexpr size_t max_extent =
        static_cast<size_t>((std::numeric_limits<ptrdiff_t>::max)());
    static constexpr bool is_mutable = true;

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        using reverse_it = std::reverse_iterator<InputIt>;

        container.clear();
        std::transform(
            reverse_it{ last }, reverse_it{ first }, std::front_inserter(container), convert_fn);
    }
};
} //namespace extenser
#endif
