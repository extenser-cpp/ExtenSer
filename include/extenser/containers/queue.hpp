// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_CONTAINERS_QUEUE_HPP
#define EXTENSER_CONTAINERS_QUEUE_HPP

#include "../extenser.hpp"
#include "deque.hpp"

#include <queue>

namespace extenser
{
template<typename Adapter, bool Deserialize, typename T, typename Container, typename Compare>
void serialize(
    serializer_base<Adapter, Deserialize>& ser, std::priority_queue<T, Container, Compare>& val)
{
    ser.as_array("", reinterpret_cast<Container&>(val));
}

template<typename Adapter, bool Deserialize, typename T, typename Container>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::queue<T, Container>& val)
{
    ser.as_array("", reinterpret_cast<Container&>(val));
}
} //namespace extenser
#endif
