// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_FILESYSTEM_HPP
#define EXTENSER_EXT_FILESYSTEM_HPP

#include "../extenser.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace extenser::detail
{
template<typename Adapter>
void serialize(serializer_base<Adapter, false>& ser, const std::filesystem::path& val)
{
    std::basic_string_view<std::filesystem::path::value_type> str = val.native();

    ser.as_string("path", str);
}

template<typename Adapter>
void serialize(serializer_base<Adapter, true>& ser, std::filesystem::path& val)
{
    std::filesystem::path::string_type str;
    ser.as_string("path", str);

    val = std::move(str);
}
} //namespace extenser::detail

#endif
