// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_EXCEPTION_HPP
#define EXTENSER_EXT_EXCEPTION_HPP

#include "../extenser.hpp"

#include <exception>
#include <string>
#include <string_view>
#include <type_traits>

namespace extenser::detail
{
template<typename Adapter>
void serialize(serializer_base<Adapter, false>& ser, const std::exception& val)
{
    std::string_view str = val.what();
    ser.as_string("what", str);
}

template<typename Adapter, typename Ex,
    typename = std::enable_if_t<std::is_base_of_v<std::exception, Ex>>>
void serialize(serializer_base<Adapter, true>& ser, Ex& val)
{
    std::string str;
    ser.as_string("what", str);

    val = Ex(str);
}
} //namespace extenser::detail

#endif
