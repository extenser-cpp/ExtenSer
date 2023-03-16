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
struct container_traits<std::array<T, N>>
{
    using container_type = std::array<T, N>;
    using size_type = typename std::array<T, N>::size_type;
    using value_type = T;
    using adapter_type = container_adapter<std::array<T, N>>;

    static constexpr bool has_fixed_size = true;
    static constexpr bool is_contiguous = true;
    static constexpr bool is_mutable = !std::is_const_v<T>;
    static constexpr bool is_sequential = true;
};

template<typename T, size_t N>
class container_adapter<std::array<T, N>> :
    public container_adapter_base<container_traits<std::array<T, N>>>
{
public:
    static constexpr auto size(const std::array<T, N>& container) -> size_t
    {
        return container.size();
    }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        std::array<T, N>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(std::distance(first, last) == N);
        std::transform(first, last, container.begin(), convert_fn);
    }
};
} //namespace extenser
#endif
