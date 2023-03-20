#ifndef CONTAINER_ADAPTERS_DEQUE_HPP
#define CONTAINER_ADAPTERS_DEQUE_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iterator>

template<typename T, typename Allocator>
class extenser::container_adapter<std::deque<T, Allocator>> :
    public container_adapter_base<container_traits<std::deque<T, Allocator>>>
{
public:
    static auto size(const std::deque<T, Allocator>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        std::deque<T, Allocator>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        container.clear();
        std::transform(first, last, std::back_inserter(container), convert_fn);
    }
};
#endif
