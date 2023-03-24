// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_UNORDERED_SET_HPP
#define EXTENSER_CONTAINERS_UNORDERED_SET_HPP

#include "../extenser.hpp"

#include <cstddef>
#include <unordered_set>

namespace extenser
{
namespace containers
{
    template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
    struct traits<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    {
        using container_type = std::unordered_set<Key, Hash, KeyEqual, Allocator>;
        using size_type = typename container_type::size_type;
        using value_type = Key;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = false;
    };

    template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
    class adapter<std::unordered_set<Key, Hash, KeyEqual, Allocator>> :
        public associative_adapter<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    {
    public:
        static auto size(const std::unordered_set<Key, Hash, KeyEqual, Allocator>& container)
            -> size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(std::unordered_set<Key, Hash, KeyEqual, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };

    template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
    struct traits<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
    {
        using container_type = std::unordered_multiset<Key, Hash, KeyEqual, Allocator>;
        using size_type = typename container_type::size_type;
        using value_type = Key;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = false;
    };

    template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
    class adapter<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>> :
        public associative_adapter<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
    {
    public:
        static auto size(const std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& container)
            -> size_t
        {
            return container.size();
        }

        template<typename Input_T, typename ConversionOp>
        static void insert_value(std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& container,
            const Input_T& value, ConversionOp convert_fn)
        {
            container.insert(convert_fn(value));
        }
    };
} //namespace containers

template<typename Adapter, bool Deserialize, typename Key, typename Hash, typename KeyEqual,
    typename Allocator>
void serialize(serializer_base<Adapter, Deserialize>& ser,
    std::unordered_set<Key, Hash, KeyEqual, Allocator>& val)
{
    ser.as_array("", val);
}

template<typename Adapter, bool Deserialize, typename Key, typename Hash, typename KeyEqual,
    typename Allocator>
void serialize(serializer_base<Adapter, Deserialize>& ser,
    std::unordered_multiset<Key, Hash, KeyEqual, Allocator>& val)
{
    ser.as_array("", val);
}
} //namespace extenser
#endif
