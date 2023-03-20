#ifndef CONTAINER_ADAPTERS_FORWARD_LIST_HPP
#define CONTAINER_ADAPTERS_FORWARD_LIST_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <iterator>

template<typename T, typename Allocator>
class extenser::container_adapter<std::forward_list<T, Allocator>> :
    public container_adapter_base<container_traits<std::forward_list<T, Allocator>>>
{
public:
    static auto size(const std::forward_list<T, Allocator>& container) -> size_t
    {
        return static_cast<size_t>(std::distance(container.begin(), container.end()));
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(std::forward_list<T, Allocator>& container, InputIt first,
        InputIt last, ConversionOp convert_fn)
    {
        using reverse_it = std::reverse_iterator<InputIt>;

        container.clear();
        std::transform(
            reverse_it{ last }, reverse_it{ first }, std::front_inserter(container), convert_fn);
    }
};
#endif
