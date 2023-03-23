// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_MAP_HPP
#define EXTENSER_CONTAINERS_MAP_HPP

#include "../extenser.hpp"

#include <cstddef>
#include <map>

namespace extenser
{
namespace containers
{
    template<typename Key, typename T, typename Compare, typename Allocator>
    struct traits<std::map<Key, T, Compare, Allocator>>
    {
        using container_type = std::map<Key, T, Compare, Allocator>;
        using key_type = Key;
        using mapped_type = T;
        using size_type = typename container_type::size_type;
        using value_type = typename container_type::value_type;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = false;
    };

    template<typename Key, typename T, typename Compare, typename Allocator>
    class adapter<std::map<Key, T, Compare, Allocator>> :
        public associative_adapter<traits<std::map<Key, T, Compare, Allocator>>>
    {
    public:
        static auto size(const std::map<Key, T, Compare, Allocator>& container) -> size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(std::map<Key, T, Compare, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };

    template<typename Key, typename T, typename Compare, typename Allocator>
    struct traits<std::multimap<Key, T, Compare, Allocator>>
    {
        using container_type = std::multimap<Key, T, Compare, Allocator>;
        using key_type = Key;
        using mapped_type = T;
        using size_type = typename container_type::size_type;
        using value_type = typename container_type::value_type;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = false;
    };

    template<typename Key, typename T, typename Compare, typename Allocator>
    class adapter<std::multimap<Key, T, Compare, Allocator>> :
        public associative_adapter<traits<std::multimap<Key, T, Compare, Allocator>>>
    {
    public:
        static auto size(const std::multimap<Key, T, Compare, Allocator>& container) -> size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(std::multimap<Key, T, Compare, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };
} //namespace containers

template<typename Adapter, bool Deserialize, typename Key, typename T, typename Compare,
    typename Allocator>
void serialize(
    serializer_base<Adapter, Deserialize>& ser, std::map<Key, T, Compare, Allocator>& val)
{
    ser.as_map("", val);
}

template<typename Adapter, bool Deserialize, typename Key, typename T, typename Compare,
    typename Allocator>
void serialize(
    serializer_base<Adapter, Deserialize>& ser, std::multimap<Key, T, Compare, Allocator>& val)
{
    ser.as_map("", val);
}
} //namespace extenser
#endif
