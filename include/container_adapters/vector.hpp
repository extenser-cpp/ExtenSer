#ifndef CONTAINER_ADAPTERS_VECTOR_HPP
#define CONTAINER_ADAPTERS_VECTOR_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

namespace extenser
{
template<typename T, typename Allocator>
struct container_traits<std::vector<T, Allocator>>
{
    using container_type = std::vector<T, Allocator>;
    using size_type = typename container_type::size_type;
    using value_type = T;
    using adapter_type = container_adapter<container_type>;

    static constexpr bool has_fixed_size = false;
    static constexpr bool is_contiguous = true;
    static constexpr bool is_mutable = true;
    static constexpr bool is_sequential = true;
};

template<typename T, typename Allocator>
class container_adapter<std::vector<T, Allocator>> :
    public container_adapter_base<container_traits<std::vector<T, Allocator>>>
{
public:
    static auto size(const std::vector<T, Allocator>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        std::vector<T, Allocator>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        container.clear();
        container.reserve(std::distance(first, last));
        std::transform(first, last, std::back_inserter(container), convert_fn);
    }
};
} //namespace extenser
#endif
