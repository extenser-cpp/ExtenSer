#ifndef CONTAINER_ADAPTERS_SET_HPP
#define CONTAINER_ADAPTERS_SET_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>

namespace extenser
{
template<typename Key, typename Compare, typename Allocator>
struct container_traits<std::set<Key, Compare, Allocator>>
{
    using container_type = std::set<Key, Compare, Allocator>;
    using size_type = typename container_type::size_type;
    using value_type = Key;
    using adapter_type = container_adapter<container_type>;

    static constexpr bool has_fixed_size = false;
    static constexpr bool is_contiguous = false;
    static constexpr bool is_mutable = true;
    static constexpr bool is_sequential = false;
};

template<typename Key, typename Compare, typename Allocator>
class container_adapter<std::set<Key, Compare, Allocator>> :
    public container_adapter_base<container_traits<std::set<Key, Compare, Allocator>>>
{
public:
    static auto size(const std::set<Key, Compare, Allocator>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(std::set<Key, Compare, Allocator>& container, InputIt first,
        InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(container.empty());
        std::transform(first, last, std::inserter(container, container.begin()), convert_fn);
    }
};

template<typename Key, typename Compare, typename Allocator>
struct container_traits<std::multiset<Key, Compare, Allocator>>
{
    using container_type = std::multiset<Key, Compare, Allocator>;
    using size_type = typename container_type::size_type;
    using value_type = Key;
    using adapter_type = container_adapter<container_type>;

    static constexpr bool has_fixed_size = false;
    static constexpr bool is_contiguous = false;
    static constexpr bool is_mutable = true;
    static constexpr bool is_sequential = false;
};

template<typename Key, typename Compare, typename Allocator>
class container_adapter<std::multiset<Key, Compare, Allocator>> :
    public container_adapter_base<container_traits<std::multiset<Key, Compare, Allocator>>>
{
public:
    static auto size(const std::multiset<Key, Compare, Allocator>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(std::multiset<Key, Compare, Allocator>& container, InputIt first,
        InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(container.empty());
        std::transform(first, last, std::inserter(container, container.begin()), convert_fn);
    }
};
} //namespace extenser
#endif
