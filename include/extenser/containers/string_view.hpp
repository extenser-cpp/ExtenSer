// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_STRING_VIEW_HPP
#define EXTENSER_CONTAINERS_STRING_VIEW_HPP

#include "../extenser.hpp"

#include <cstddef>
#include <string_view>

namespace extenser
{
namespace containers
{
    template<typename CharT, typename Traits>
    struct traits<std::basic_string_view<CharT, Traits>>
    {
        using container_type = std::basic_string<CharT, Traits>;
        using character_type = CharT;
        using size_type = typename container_type::size_type;
        using value_type = typename container_type::value_type;
        using adapter_type = adapter<container_type>;

        static constexpr bool has_fixed_size = true;
        static constexpr bool is_contiguous = true;
        static constexpr bool is_mutable = false;
        static constexpr bool is_sequential = true;
    };

    template<typename CharT, typename Traits>
    class adapter<std::basic_string_view<CharT, Traits>> :
        public string_adapter<std::basic_string_view<CharT, Traits>>
    {
    public:
        static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>,
            "Unsupported character type detected");

        using container_type = std::basic_string_view<CharT, Traits>;

        static auto size(const container_type& container) -> size_t { return container.size(); }

        template<typename InputIt, typename ConversionOp>
        static void assign_from_range([[maybe_unused]] container_type& container,
            [[maybe_unused]] InputIt first, [[maybe_unused]] InputIt last,
            [[maybe_unused]] ConversionOp convert_fn)
        {
            // nop
        }
    };
} //namespace containers

template<typename Adapter, typename CharT, typename Traits>
void serialize(
    serializer_base<Adapter, false>& ser, const std::basic_string_view<CharT, Traits> val)
{
    ser.as_string("", val);
}

template<typename Adapter, typename CharT, typename Traits>
void serialize([[maybe_unused]] serializer_base<Adapter, true>& ser,
    [[maybe_unused]] std::basic_string_view<CharT, Traits>& val)
{
    // nop
}
} //namespace extenser
#endif
