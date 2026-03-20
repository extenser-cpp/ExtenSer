// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_BITSET_HPP
#define EXTENSER_EXT_BITSET_HPP

#include "../extenser.hpp"

#include <bitset>
#include <string>

namespace extenser::detail
{
template<typename Adapter, std::size_t N>
void serialize(serializer_base<Adapter, false>& ser, const std::bitset<N>& val)
{
    if constexpr (N <= sizeof(unsigned long long) * 8)
    {
        const auto n = val.to_ullong();
        ser.as_uint("bits", n);
    }
    else
    {
        const auto str = val.to_string();
        ser.as_string("bits", str);
    }
}

template<typename Adapter, std::size_t N>
void serialize(serializer_base<Adapter, true>& ser, std::bitset<N>& val)
{
    if constexpr (N <= sizeof(unsigned long long) * 8)
    {
        unsigned long long n;
        ser.as_uint("bits", n);

        val = std::bitset<N>(n);
    }
    else
    {
        std::string str;
        ser.as_string("bits", str);

        val = std::bitset<N>(str);
    }
}
} // namespace extenser::detail

#endif
