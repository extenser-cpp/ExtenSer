// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_SPAN_HPP
#define EXTENSER_CONTAINERS_SPAN_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <type_traits>

#if __cplusplus >= 202002L
#  include <span>
#endif

namespace extenser
{
namespace containers
{
#if defined(__cpp_lib_span)
    template<typename T, std::size_t N>
    struct traits<span<T, N>>
    {
        using container_type = span<T, N>;
        using size_type = typename span<T, N>::size_type;
        using value_type = T;
        using adapter_type = adapter<span<T, N>>;

        static constexpr bool has_fixed_size = true;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = !std::is_const_v<T>;
        static constexpr bool is_sequential = true;
    };

    template<typename T, std::size_t N>
    class adapter<span<T, N>> : public sequential_adapter<span<T, N>>
    {
    public:
        static constexpr auto size(const span<T, N>& container) -> std::size_t
        {
            return container.size();
        }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(
            span<T, N>& container, InputIt first, InputIt last, ConversionOp convert_fn)
        {
            EXTENSER_PRECONDITION(std::distance(first, last) >= 0
                && static_cast<std::size_t>(std::distance(first, last)) == container.size());

            std::transform(first, last, container.begin(), convert_fn);
        }
    };
#else
    template<typename T>
    struct traits<span<T>>
    {
        using container_type = span<T>;
        using size_type = typename span<T>::size_type;
        using value_type = T;
        using adapter_type = adapter<span<T>>;

        static constexpr bool has_fixed_size = true;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = !std::is_const_v<T>;
        static constexpr bool is_sequential = true;
    };

    template<typename T>
    class adapter<span<T>> : public sequential_adapter<span<T>>
    {
    public:
        static constexpr auto size(const span<T>& container) -> std::size_t
        {
            return container.size();
        }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(
            span<T>& container, InputIt first, InputIt last, ConversionOp convert_fn)
        {
            EXTENSER_PRECONDITION(std::distance(first, last) >= 0
                && static_cast<std::size_t>(std::distance(first, last)) == container.size());

            std::transform(first, last, container.begin(), convert_fn);
        }
    };
#endif
} //namespace containers

namespace detail
{
#if defined(__cpp_lib_span)
    template<typename Adapter, bool Deserialize, typename T, std::size_t N>
    void serialize(serializer_base<Adapter, Deserialize>& ser, span<T, N>& val)
    {
        ser.as_array("", val);
    }
#else
    template<typename Adapter, bool Deserialize, typename T>
    void serialize(serializer_base<Adapter, Deserialize>& ser, span<T>& val)
    {
        ser.as_array("", val);
    }
#endif
} //namespace detail
} //namespace extenser
#endif
