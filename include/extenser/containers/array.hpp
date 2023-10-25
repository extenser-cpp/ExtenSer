// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_ARRAY_HPP
#define EXTENSER_CONTAINERS_ARRAY_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

namespace extenser
{
namespace containers
{
    template<typename T, std::size_t N>
    struct traits<std::array<T, N>>
    {
        using container_type = std::array<T, N>;
        using size_type = typename std::array<T, N>::size_type;
        using value_type = T;
        using adapter_type = adapter<std::array<T, N>>;

        static constexpr bool has_fixed_size = true;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = !std::is_const_v<T>;
        static constexpr bool is_sequential = true;
    };

    template<typename T, std::size_t N>
    class adapter<std::array<T, N>> : public sequential_adapter<std::array<T, N>>
    {
    public:
        static constexpr auto size(const std::array<T, N>& container) -> std::size_t
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
} //namespace containers

namespace detail
{
    template<typename Adapter, bool Deserialize, typename T, std::size_t N>
    void serialize(serializer_base<Adapter, Deserialize>& ser, std::array<T, N>& val)
    {
        span arr_span{ val };
        ser.as_array("", arr_span);
    }
} //namespace detail
} //namespace extenser
#endif
