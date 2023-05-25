// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_STRING_HPP
#define EXTENSER_CONTAINERS_STRING_HPP

#include "../extenser.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace extenser
{
namespace containers
{
    template<typename CharT, typename Traits, typename Allocator>
    struct traits<std::basic_string<CharT, Traits, Allocator>>
    {
        using container_type = std::basic_string<CharT, Traits, Allocator>;
        using character_type = CharT;
        using size_type = typename container_type::size_type;
        using value_type = typename container_type::value_type;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = true;
    };

    template<typename CharT, typename Traits, typename Allocator>
    class adapter<std::basic_string<CharT, Traits, Allocator>> :
        public string_adapter<std::basic_string<CharT, Traits, Allocator>>
    {
    public:
        static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>
                || std::is_same_v<CharT, char16_t> || std::is_same_v<CharT, char32_t>
#if defined(__cpp_char8_t)
                || std::is_same_v<CharT, char8_t>
#endif
            ,
            "Unsupported character type detected");

        using container_type = std::basic_string<CharT, Traits, Allocator>;

        static auto size(const container_type& container) -> size_t { return container.size(); }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range(
            container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
        {
            container.clear();
            container.reserve(std::distance(first, last));
            std::transform(first, last, std::back_inserter(container), convert_fn);
        }

        static auto to_string(const container_type& container) -> std::string
        {
            if constexpr (std::is_same_v<CharT, char>)
            {
                return container;
            }
            else
            {
                return { container.begin(), container.end() };
            }
        }

        static auto to_string(container_type&& container) -> std::string
        {
            if constexpr (std::is_same_v<CharT, char>)
            {
                return std::move(container);
            }
            else
            {
                return { container.begin(), container.end() };
            }
        }

        static auto from_string(const std::string& str) -> container_type
        {
            if constexpr (std::is_same_v<CharT, char>)
            {
                return str;
            }
            else
            {
                return { str.begin(), str.end() };
            }
        }

        static auto from_string(std::string&& str) -> container_type
        {
            if constexpr (std::is_same_v<CharT, char>)
            {
                return std::move(str);
            }
            else
            {
                return { str.begin(), str.end() };
            }
        }
    };
} //namespace containers

template<typename Adapter, bool Deserialize, typename CharT, typename Traits, typename Allocator>
void serialize(
    serializer_base<Adapter, Deserialize>& ser, std::basic_string<CharT, Traits, Allocator>& val)
{
    ser.as_string("", val);
}
} //namespace extenser
#endif
