#ifndef CONTAINER_ADAPTERS_UNORDERED_SET_HPP
#define CONTAINER_ADAPTERS_UNORDERED_SET_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <unordered_set>

namespace extenser
{
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct container_adapter<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
{
    using container_type = std::unordered_set<Key, Hash, KeyEqual, Allocator>;
    using value_type = typename container_type::value_type;

    static constexpr bool has_dynamic_extent = true;
    static constexpr size_t max_extent =
        static_cast<size_t>((std::numeric_limits<ptrdiff_t>::max)());
    static constexpr bool is_mutable = true;

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        container.clear();
        std::transform(first, last, std::inserter(container, container.begin()), convert_fn);
    }
};

template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
struct container_adapter<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
{
    using container_type = std::unordered_multiset<Key, Hash, KeyEqual, Allocator>;
    using value_type = typename container_type::value_type;

    static constexpr bool has_dynamic_extent = true;
    static constexpr size_t max_extent =
        static_cast<size_t>((std::numeric_limits<ptrdiff_t>::max)());
    static constexpr bool is_mutable = true;

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        container.clear();
        std::transform(first, last, std::inserter(container, container.begin()), convert_fn);
    }
};
} //namespace extenser
#endif
