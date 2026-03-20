// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_FORWARD_LIST_HPP
#define EXTENSER_CONTAINERS_FORWARD_LIST_HPP

#include "../extenser.hpp"

#include <cstddef>
#include <forward_list>
#include <iterator>

namespace extenser
{
namespace containers
{
    template<typename T, typename Allocator>
    struct traits<std::forward_list<T, Allocator>> :
        sequential_traits<std::forward_list<T, Allocator>, false, false, true>
    {
    };

    template<typename T, typename Allocator>
    class adapter<std::forward_list<T, Allocator>> :
        public sequential_adapter<std::forward_list<T, Allocator>>
    {
    public:
        static auto size(const std::forward_list<T, Allocator>& container) -> std::size_t
        {
            return static_cast<std::size_t>(std::distance(container.begin(), container.end()));
        }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(std::forward_list<T, Allocator>& container, InputIt first,
            InputIt last, ConversionOp convert_fn)
        {
            container.clear();

            auto tail = container.before_begin();

            for (; first != last; ++first)
            {
                tail = container.emplace_after(tail, convert_fn(*first));
            }
        }
    };
} //namespace containers

namespace detail
{
    template<typename Adapter, bool Deserialize, typename T, typename Allocator>
    void serialize(serializer_base<Adapter, Deserialize>& ser, std::forward_list<T, Allocator>& val)
    {
        ser.as_array("", val);
    }
} //namespace detail
} //namespace extenser
#endif
