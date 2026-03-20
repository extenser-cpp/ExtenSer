// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_UNORDERED_MAP_HPP
#define EXTENSER_CONTAINERS_UNORDERED_MAP_HPP

#include "../extenser.hpp"

#include <cstddef>
#include <unordered_map>

namespace extenser
{
namespace containers
{
    template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    struct traits<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> :
        map_traits<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>, false, true>
    {
    };

    template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    class adapter<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> :
        public associative_adapter<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
    {
    public:
        static auto size(const std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& container)
            -> std::size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };

    template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    struct traits<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>> :
        map_traits<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>, false, true>
    {
    };

    template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    class adapter<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>> :
        public associative_adapter<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>
    {
    public:
        static auto size(
            const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& container)
            -> std::size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(
            std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };
} //namespace containers

namespace detail
{
    template<typename Adapter, bool Deserialize, typename Key, typename T, typename Hash,
        typename KeyEqual, typename Allocator>
    void serialize(serializer_base<Adapter, Deserialize>& ser,
        std::unordered_map<Key, T, Hash, KeyEqual, Allocator>& val)
    {
        ser.as_map("", val);
    }

    template<typename Adapter, bool Deserialize, typename Key, typename T, typename Hash,
        typename KeyEqual, typename Allocator>
    void serialize(serializer_base<Adapter, Deserialize>& ser,
        std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>& val)
    {
        ser.as_map("", val);
    }
} //namespace detail
} //namespace extenser
#endif
