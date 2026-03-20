// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_ATOMIC_HPP
#define EXTENSER_EXT_ATOMIC_HPP

#include "../extenser.hpp"

#include <atomic>

namespace extenser::detail
{
template<typename Adapter, typename T>
void serialize(serializer_base<Adapter, false>& ser, const std::atomic<T>& val)
{
    auto value = val.load(std::memory_order_acquire);

    ser.as_object("value", value);
}

template<typename Adapter, typename T>
void serialize(serializer_base<Adapter, true>& ser, std::atomic<T>& val)
{
    T value;
    ser.as_object("value", value);

    val.store(value, std::memory_order_release);
}
} //namespace extenser::detail

#endif
