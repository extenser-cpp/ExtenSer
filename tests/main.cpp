#include "bitsery/extenser_bitsery.hpp"
#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <string>

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
