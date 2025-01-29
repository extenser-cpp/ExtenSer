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
#include "extenser/containers/map.hpp"
#include "extenser/containers/queue.hpp"
#include "extenser/containers/span.hpp"
#include "extenser/containers/stack.hpp"
#include "extenser/containers/string.hpp"
#include "extenser/containers/vector.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <numeric>
#include <queue>
#include <stack>
#include <string>
#include <vector>

namespace
{
struct SimplePerson
{
    int age{};
    std::string name{};

    template<typename S>
    void serialize(S& ser)
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
#if defined(__clang__) && __clang_major__ >= 16
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

    int arr[200];
    std::iota(std::begin(arr), std::end(arr), 0);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(arr);

    std::fill(std::begin(arr), std::end(arr), 0);
    ser.deserialize_object(arr);

    REQUIRE_EQ(arr[0], 0);
    REQUIRE_EQ(arr[199], 199);

#if defined(__clang__) && __clang_major__ >= 16
#  pragma clang diagnostic pop
#endif
}

TEST_CASE("C++ Array")
{
    std::array<int, 50> fix_arr; // NOLINT(*-pro-type-member-init)
    std::iota(fix_arr.begin(), fix_arr.end(), 0);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(fix_arr);

    std::fill(fix_arr.begin(), fix_arr.end(), 0);

    ser.deserialize_object(fix_arr);

    REQUIRE_EQ(fix_arr[0], 0);
    REQUIRE_EQ(fix_arr[49], 49);
}

TEST_CASE("Span")
{
    std::vector<int> dyn_arr(100);
    std::iota(dyn_arr.begin(), dyn_arr.end(), 0);

    span<int> span(dyn_arr.begin(), 50);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(span);

    std::fill(dyn_arr.begin(), dyn_arr.end(), 0);

    ser.deserialize_object(span);

    REQUIRE_EQ(span[0], 0);
    REQUIRE_EQ(span[49], 49);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 49);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("P. Queue")
{
    std::priority_queue<int> queue;

    queue.push(1);
    queue.push(9);
    queue.push(-1);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(queue);

    ser.deserialize_object(queue);

    REQUIRE_EQ(queue.top(), 9);
}

TEST_CASE("Queue")
{
    std::queue<int> queue;

    queue.push(1);
    queue.push(9);
    queue.push(-1);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(queue);

    ser.deserialize_object(queue);

    REQUIRE_EQ(queue.size(), 3);
    REQUIRE_EQ(queue.front(), 1);
    REQUIRE_EQ(queue.back(), -1);
}

TEST_CASE("Stack")
{
    std::stack<int, std::vector<int>> stk;
    stk.push(5);
    stk.push(4);
    stk.push(-1);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(stk);

    ser.deserialize_object(stk);

    REQUIRE_EQ(stk.size(), 3);
    REQUIRE_EQ(stk.top(), -1);
}

TEST_CASE("Vector")
{
    std::vector<int> nvec{ 1, 2, 3, 4, 5 };

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(nvec);

    ser.deserialize_object(nvec);

    REQUIRE_EQ(nvec.size(), 5);
    REQUIRE_EQ(nvec.at(4), 5);
}

TEST_CASE("View")
{
    std::vector<int> dyn_arr(100);
    std::iota(dyn_arr.begin(), dyn_arr.end(), 0);

    view<int> view(dyn_arr.begin(), 50);

    easy_serializer<json_adapter> ser{};
    ser.serialize_object(view);

    std::fill(dyn_arr.begin(), dyn_arr.end(), 0);

    ser.deserialize_object(view);

    REQUIRE_EQ(view[0], 0);
    REQUIRE_EQ(view[49], 0);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 0);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("Simple JSON serialize/deserialize")
{
    const SimplePerson in_person{ 42, "Jake" };
    easy_serializer<json_adapter> ser{};
    ser.serialize_object(in_person);

    SimplePerson out_person{};

    ser.deserialize_object(out_person);

    REQUIRE_EQ(out_person.age, 42);
    REQUIRE_EQ(out_person.name, "Jake");
}

struct NoDefault
{
    NoDefault() = delete;
    explicit NoDefault(int num) : number(num) {}

    template<typename S>
    void serialize(extenser::generic_serializer<S>& ser)
    {
        ser.as_int("", number);
    }

    int number;
};

TEST_CASE("README Example")
{
    extenser::easy_serializer<json_adapter> serializer{};

    // Serialize default constructible type
    const std::string input_str = "Hello, world!";
    serializer.serialize_object(input_str);

    const auto output_str = serializer.deserialize_object<std::string>();
    REQUIRE(!output_str.empty());
    REQUIRE_EQ(output_str, input_str);

    // Serialize non-default constructible type
    NoDefault input_nd(2);
    serializer.serialize_object(input_nd);

    NoDefault out_nd(1);
    serializer.deserialize_object(out_nd);
    REQUIRE_EQ(out_nd.number, 2);
}

TEST_CASE("New syntax")
{
    easy_serializer<json_adapter> serializer{};
    const auto& json = serializer.object();

    REQUIRE(json.is_null());

    const std::string input_str = "Hello, world!";

    // Serialize one object (overwrites existing serialized data)
    serializer.serialize_object(input_str);

    REQUIRE(json.is_string());

    // Deserialize one object
    const auto output_str = serializer.deserialize_object<std::string>();

    REQUIRE_FALSE(output_str.empty());
    CHECK_EQ(output_str, input_str);

    SUBCASE("Serializing overwrites")
    {
        REQUIRE(json.is_string());

        const std::vector<float> input_vec{ 0.1f, 0.2f, 0.3f };
        serializer.serialize_object(input_vec);

        REQUIRE(json.is_array());

        const auto output_vec = serializer.deserialize_object<std::vector<float>>();

        REQUIRE_FALSE(output_vec.empty());
        CHECK_EQ(output_vec, input_vec);
    }
}

TEST_CASE("'Quick' methods")
{
    const std::string input_str = "Hello, world!";

    const auto json = easy_serializer<json_adapter>::quick_serialize(input_str);

    REQUIRE(json.is_string());

    const auto output_str = easy_serializer<json_adapter>::quick_deserialize<std::string>(json);

    REQUIRE_FALSE(output_str.empty());
    CHECK_EQ(output_str, input_str);
}
} //namespace extenser::tests
