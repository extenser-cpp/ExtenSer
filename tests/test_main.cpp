// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#include "extenser/extenser.hpp"

#include "extenser/json_adapter/extenser_json.hpp"
#include "extenser/containers/array.hpp"
#include "extenser/containers/queue.hpp"
#include "extenser/containers/span.hpp"
#include "extenser/containers/stack.hpp"
#include "extenser/containers/vector.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct SimplePerson
{
    int age{};
    std::string name{};

    template<typename S>
    void serialize(extenser::generic_serializer<S>& ser)
    {
        ser.as_int("age", age);
        ser.as_string("name", name);
    }
};

static_assert(extenser::is_object_serializable<SimplePerson>, "Person is not serializable");
} //namespace

namespace extenser::tests
{
TEST_CASE("C-Array")
{
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

    int arr[200];
    std::iota(std::begin(arr), std::end(arr), 0);

    serializer<json_adapter> ser{};
    ser.serialize_object(arr);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 200);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[199], 199);

    std::fill(std::begin(arr), std::end(arr), 0);
    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(arr);

    REQUIRE_EQ(arr[0], 0);
    REQUIRE_EQ(arr[199], 199);

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
}

TEST_CASE("C++ Array")
{
    std::array<int, 50> fix_arr;
    std::iota(fix_arr.begin(), fix_arr.end(), 0);

    serializer<json_adapter> ser{};
    ser.serialize_object(fix_arr);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(fix_arr.begin(), fix_arr.end(), 0);
    obj[49] = 52;

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(fix_arr);

    REQUIRE_EQ(fix_arr[0], 0);
    REQUIRE_EQ(fix_arr[49], 52);
}

TEST_CASE("Span")
{
    std::vector<int> dyn_arr(100);
    std::iota(dyn_arr.begin(), dyn_arr.end(), 0);

    span<int> span(dyn_arr.begin(), 50);

    serializer<json_adapter> ser{};
    ser.serialize_object(span);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(dyn_arr.begin(), dyn_arr.end(), 0);
    obj[49] = 52;

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(span);

    REQUIRE_EQ(span[0], 0);
    REQUIRE_EQ(span[49], 52);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 52);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("P. Queue")
{
    std::priority_queue<int> queue;

    queue.push(1);
    queue.push(9);
    queue.push(-1);

    serializer<json_adapter> ser{};
    ser.serialize_object(queue);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 3);

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(queue);

    REQUIRE_EQ(queue.top(), 9);
}

TEST_CASE("Queue")
{
    std::queue<int> queue;

    queue.push(1);
    queue.push(9);
    queue.push(-1);

    serializer<json_adapter> ser{};
    ser.serialize_object(queue);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 3);

    obj.insert(obj.begin(), 3);
    REQUIRE_EQ(obj.size(), 4);

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(queue);

    REQUIRE_EQ(queue.size(), 4);
    REQUIRE_EQ(queue.front(), 3);
}

TEST_CASE("Stack")
{
    std::stack<int, std::vector<int>> stk;
    stk.push(5);
    stk.push(4);
    stk.push(-1);

    serializer<json_adapter> ser{};
    ser.serialize_object(stk);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 3);

    obj.push_back(3);
    REQUIRE_EQ(obj.size(), 4);

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(stk);

    REQUIRE_EQ(stk.size(), 4);
    REQUIRE_EQ(stk.top(), 3);
}

TEST_CASE("Vector")
{
    std::vector<int> nvec{ 1, 2, 3, 4, 5 };

    serializer<json_adapter> ser{};
    ser.serialize_object(nvec);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 5);

    obj.push_back(6);
    REQUIRE_EQ(obj.size(), 6);

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(nvec);

    REQUIRE_EQ(nvec.size(), 6);
    REQUIRE_EQ(nvec.at(5), 6);
}

TEST_CASE("View")
{
    std::vector<int> dyn_arr(100);
    std::iota(dyn_arr.begin(), dyn_arr.end(), 0);

    view<int> view(dyn_arr.begin(), 50);

    serializer<json_adapter> ser{};
    ser.serialize_object(view);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(dyn_arr.begin(), dyn_arr.end(), 0);
    obj[49] = 52;

    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(view);

    REQUIRE_EQ(view[0], 0);
    REQUIRE_EQ(view[49], 0);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 0);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("Simple JSON serialize/deserialize")
{
    const SimplePerson in_person{ 42, "Jake" };
    serializer<json_adapter> ser{};
    ser.serialize_object(in_person);

    auto obj = std::move(ser).object();

    REQUIRE(obj.contains("age"));
    REQUIRE_EQ(obj["age"], 42);
    REQUIRE(obj.contains("name"));
    REQUIRE_EQ(obj["name"], "Jake");

    SimplePerson out_person{};
    deserializer<json_adapter> dser{ obj };
    dser.deserialize_object(out_person);

    REQUIRE_EQ(out_person.age, 42);
    REQUIRE_EQ(out_person.name, "Jake");
}
} //namespace extenser::tests
