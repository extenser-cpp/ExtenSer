// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2026 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_EXT_COMPLEX_HPP
#define EXTENSER_EXT_COMPLEX_HPP

#include "../extenser.hpp"

#include <complex>

namespace extenser::detail
{
template<typename Adapter, typename T>
void serialize(serializer_base<Adapter, false>& ser, const std::complex<T> val)
{
    T real = val.real();
    T imag = val.imag();

    ser.as_float("real", real);
    ser.as_float("imag", imag);
}

template<typename Adapter, typename T>
void serialize(serializer_base<Adapter, true>& ser, std::complex<T>& val)
{
    T real;
    T imag;

    ser.as_float("real", real);
    ser.as_float("imag", imag);

    val = std::complex<T>(real, imag);
}
} //namespace extenser::detail

#endif
