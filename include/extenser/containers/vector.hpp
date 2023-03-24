// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_VECTOR_HPP
#define EXTENSER_CONTAINERS_VECTOR_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

namespace extenser
{
namespace containers
{
    template<typename T, typename Allocator>
    struct traits<std::vector<T, Allocator>>
    {
        using container_type = std::vector<T, Allocator>;
        using size_type = typename container_type::size_type;
        using value_type = T;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = true;
    };

    template<typename T, typename Allocator>
    class adapter<std::vector<T, Allocator>> :
        public sequential_adapter<traits<std::vector<T, Allocator>>>
    {
    public:
        static auto size(const std::vector<T, Allocator>& container) -> size_t
        {
            return container.size();
        }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(std::vector<T, Allocator>& container, InputIt first,
            InputIt last, ConversionOp convert_fn)
        {
            container.clear();
            container.reserve(std::distance(first, last));
            std::transform(first, last, std::back_inserter(container), convert_fn);
        }
    };
} //namespace containers

template<typename Adapter, bool Deserialize, typename T, typename Allocator>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::vector<T, Allocator>& val)
{
    ser.as_array("", val);
}
} //namespace extenser
#endif
