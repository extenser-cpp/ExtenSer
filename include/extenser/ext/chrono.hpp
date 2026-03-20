// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_CHRONO_HPP
#define EXTENSER_EXT_CHRONO_HPP

#include "../extenser.hpp"

#include <chrono>

namespace extenser::detail
{
template<typename Adapter, typename Rep, typename Period>
void serialize(serializer_base<Adapter, false>& ser, const std::chrono::duration<Rep, Period> val)
{
    Rep count = val.count();

    if constexpr (std::is_integral_v<Rep>)
    {
        if constexpr (std::is_signed_v<Rep>)
        {
            ser.as_int("count", count);
        }
        else
        {
            ser.as_uint("count", count);
        }
    }
    else
    {
        ser.as_float("count", count);
    }
}

template<typename Adapter, typename Rep, typename Period>
void serialize(serializer_base<Adapter, true>& ser, std::chrono::duration<Rep, Period>& val)
{
    Rep count;

    if constexpr (std::is_integral_v<Rep>)
    {
        if constexpr (std::is_signed_v<Rep>)
        {
            ser.as_int("count", count);
        }
        else
        {
            ser.as_uint("count", count);
        }
    }
    else
    {
        ser.as_float("count", count);
    }

    val = std::chrono::duration<Rep, Period>(count);
}

template<typename Adapter, typename Clock, typename Duration>
void serialize(
    serializer_base<Adapter, false>& ser, const std::chrono::time_point<Clock, Duration> val)
{
    auto since_epoch = val.time_since_epoch();
    serialize(ser, since_epoch);
}

template<typename Adapter, typename Clock, typename Duration>
void serialize(serializer_base<Adapter, true>& ser, std::chrono::time_point<Clock, Duration>& val)
{
    Duration since_epoch;
    serialize(ser, since_epoch);

    val = std::chrono::time_point<Clock, Duration>(since_epoch);
}
} //namespace extenser::detail

#endif
