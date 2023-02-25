#include "extenser.hpp"

#include "bitsery/extenser_bitsery.hpp"
#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

const size_t extenser::bitsery_adapter::config::max_container_size = 1'000;
const size_t extenser::bitsery_adapter::config::max_string_size = 1'000;

namespace
{
struct Person
{
    int age{};
    std::string name{};
};

template<typename S>
void serialize(extenser::generic_serializer<S>& ser, Person& person)
{
    ser.as_int("age", person.age);
    ser.as_string("name", person.name);
}
} //namespace

TEST_CASE("C-Array")
{
    int arr[200];
    std::iota(std::begin(arr), std::end(arr), 0);

    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(arr);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 200);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[199], 199);

    std::fill(std::begin(arr), std::end(arr), 0);
    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(arr);

    REQUIRE_EQ(arr[0], 0);
    REQUIRE_EQ(arr[199], 199);
}

TEST_CASE("C++ Array")
{
    std::array<int, 50> fix_arr;
    std::iota(fix_arr.begin(), fix_arr.end(), 0);

    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(fix_arr);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(fix_arr.begin(), fix_arr.end(), 0);
    obj[49] = 52;

    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(fix_arr);

    REQUIRE_EQ(fix_arr[0], 0);
    REQUIRE_EQ(fix_arr[49], 52);
}

TEST_CASE("Span")
{
    auto dyn_arr = std::make_unique<int[]>(100);
    std::iota(dyn_arr.get(), std::next(dyn_arr.get(), 100), 0);

    extenser::span<int> span(dyn_arr.get(), 50);

    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(span);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(dyn_arr.get(), std::next(dyn_arr.get(), 100), 0);
    obj[49] = 52;

    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(span);

    REQUIRE_EQ(span[0], 0);
    REQUIRE_EQ(span[49], 52);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 52);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("Vector")
{
    std::vector<int> nvec{ 1, 2, 3, 4, 5 };

    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(nvec);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 5);

    obj.push_back(6);
    REQUIRE_EQ(obj.size(), 6);

    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(nvec);

    REQUIRE_EQ(nvec.size(), 6);
    REQUIRE_EQ(nvec.at(5), 6);
}

TEST_CASE("View")
{
    auto dyn_arr = std::make_unique<int[]>(100);
    std::iota(dyn_arr.get(), std::next(dyn_arr.get(), 100), 0);

    extenser::view<int> view(dyn_arr.get(), 50);

    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(view);

    auto obj = std::move(ser).object();

    REQUIRE(obj.is_array());
    REQUIRE_EQ(obj.size(), 50);
    REQUIRE_EQ(obj[0], 0);
    REQUIRE_EQ(obj[49], 49);

    std::fill(dyn_arr.get(), std::next(dyn_arr.get(), 100), 0);
    obj[49] = 52;

    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(view);

    REQUIRE_EQ(view[0], 0);
    REQUIRE_EQ(view[49], 0);
    REQUIRE_EQ(dyn_arr[0], 0);
    REQUIRE_EQ(dyn_arr[49], 0);
    REQUIRE_EQ(dyn_arr[99], 0);
}

TEST_CASE("Simple JSON serialize/deserialize")
{
    const Person in_person{ 42, "Jake" };
    extenser::serializer<extenser::json_adapter> ser{};
    ser.serialize_object(in_person);

    auto obj = std::move(ser).object();

    REQUIRE(obj.contains("age"));
    REQUIRE_EQ(obj["age"], 42);
    REQUIRE(obj.contains("name"));
    REQUIRE_EQ(obj["name"], "Jake");

    Person out_person{};
    extenser::deserializer<extenser::json_adapter> dser{ obj };
    dser.deserialize_object(out_person);

    REQUIRE_EQ(out_person.age, 42);
    REQUIRE_EQ(out_person.name, "Jake");
}

TEST_CASE("Simple bitsery serialize/deserialize")
{
    const Person in_person{ 42, "Jake" };
    extenser::serializer<extenser::bitsery_adapter> ser{};
    ser.serialize_object(in_person);

    auto obj = std::move(ser).object();

    Person out_person{};
    extenser::deserializer<extenser::bitsery_adapter> dser{ obj };
    dser.deserialize_object(out_person);

    REQUIRE_EQ(out_person.age, 42);
    REQUIRE_EQ(out_person.name, "Jake");
}
