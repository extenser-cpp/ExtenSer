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

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <iterator>

namespace extenser
{
namespace containers
{
    template<typename T, typename Allocator>
    struct traits<std::forward_list<T, Allocator>>
    {
        using container_type = std::forward_list<T, Allocator>;
        using size_type = typename container_type::size_type;
        using value_type = T;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = true;
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
            using reverse_it = std::reverse_iterator<InputIt>;

            container.clear();
            std::transform(reverse_it{ last }, reverse_it{ first }, std::front_inserter(container),
                convert_fn);
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
