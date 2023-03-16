#ifndef CONTAINER_ADAPTERS_LIST_HPP
#define CONTAINER_ADAPTERS_LIST_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>

template<typename T, typename Allocator>
class extenser::container_adapter<std::list<T, Allocator>> :
    public container_adapter_base<container_traits<std::list<T, Allocator>>>
{
public:
    static auto size(const std::list<T, Allocator>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        std::list<T, Allocator>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(container.empty());
        std::transform(first, last, std::back_inserter(container), convert_fn);
    }
};
#endif
