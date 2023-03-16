#ifndef CONTAINER_ADAPTERS_SPAN_HPP
#define CONTAINER_ADAPTERS_SPAN_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <type_traits>

#if defined(__cpp_lib_span)
#  include <span>
#endif

namespace extenser
{
#if defined(__cpp_lib_span)
template<typename T, size_t N>
struct container_traits<span<T, N>>
{
    using container_type = span<T, N>;
    using size_type = typename span<T, N>::size_type;
    using value_type = T;
    using adapter_type = container_adapter<span<T, N>>;

    static constexpr bool has_fixed_size = true;
    static constexpr bool is_contiguous = true;
    static constexpr bool is_mutable = !std::is_const_v<T>;
    static constexpr bool is_sequential = true;
};

template<typename T, size_t N>
class container_adapter<span<T, N>> : public container_adapter_base<span_traits<T, N>>
{
public:
    static constexpr auto size(const span<T, N>& container) -> size_t { return container.size(); }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        span<T, N>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        std::transform(first, last, container.begin(), convert_fn);
    }
};
#else
template<typename T>
struct container_traits<span<T>>
{
    using container_type = span<T>;
    using size_type = typename span<T>::size_type;
    using value_type = T;
    using adapter_type = container_adapter<span<T>>;

    static constexpr bool has_fixed_size = true;
    static constexpr bool is_contiguous = true;
    static constexpr bool is_mutable = !std::is_const_v<T>;
    static constexpr bool is_sequential = true;
};

template<typename T>
class container_adapter<span<T>> : public container_adapter_base<container_traits<span<T>>>
{
public:
    static constexpr auto size(const span<T>& container) -> size_t { return container.size(); }

    template<typename InputIt, typename ConversionOp>
    static void assign_from_range(
        span<T>& container, InputIt first, InputIt last, ConversionOp convert_fn)
    {
        EXTENSER_PRECONDITION(std::distance(first, last) == container.size());
        std::transform(first, last, container.begin(), convert_fn);
    }
};
#endif
} //namespace extenser
#endif
