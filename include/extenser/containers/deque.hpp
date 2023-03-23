// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_DEQUE_HPP
#define EXTENSER_CONTAINERS_DEQUE_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iterator>

namespace extenser
{
namespace containers
{
    template<typename T, typename Allocator>
    class adapter<std::deque<T, Allocator>> :
        public sequential_adapter<traits<std::deque<T, Allocator>>>
    {
    public:
        static auto size(const std::deque<T, Allocator>& container) -> size_t
        {
            return container.size();
        }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(std::deque<T, Allocator>& container, InputIt first,
            InputIt last, ConversionOp convert_fn)
        {
            container.clear();
            std::transform(first, last, std::back_inserter(container), convert_fn);
        }
    };
} //namespace containers

template<typename Adapter, bool Deserialize, typename T, typename Allocator>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::deque<T, Allocator>& val)
{
    ser.as_array("", val);
}
} //namespace extenser
#endif
