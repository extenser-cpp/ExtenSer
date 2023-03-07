#include "json.test.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace extenser::tests
{
TEST_SUITE("json::deserializer")
{
    using deserializer = json_adapter::deserializer_t;

    TEST_CASE("CTOR")
    {
        const nlohmann::json obj{};
        std::optional<deserializer> dser{};
        REQUIRE_FALSE(dser.has_value());

        REQUIRE_NOTHROW(dser.emplace(obj));
        REQUIRE(dser.has_value());
    }

    TEST_CASE("as_bool")
    {
        const nlohmann::json test_obj1 = nlohmann::json::parse("false");
        const nlohmann::json test_obj2 = nlohmann::json::parse(R"({"test_val": true})");

        std::optional<deserializer> dser{ test_obj1 };

        bool test_val{ true };
        REQUIRE_NOTHROW(dser->as_bool("", test_val));
        REQUIRE_FALSE(test_val);

        dser.emplace(test_obj2);

        REQUIRE_NOTHROW(dser->as_bool("test_val", test_val));
        REQUIRE(test_val);
    }

    TEST_CASE("as_float")
    {
        const nlohmann::json test_obj1 = nlohmann::json::parse("1.256");
        const nlohmann::json test_obj2 = nlohmann::json::parse(R"({"test_val": 112E-6})");
        const nlohmann::json test_obj3 = NAN;

        std::optional<deserializer> dser{ test_obj1 };

        float test_float{};
        REQUIRE_NOTHROW(dser->as_float("", test_float));
        REQUIRE_EQ(static_cast<double>(test_float), doctest::Approx(1.256).epsilon(0.0001));

        dser.emplace(test_obj2);

        double test_double{};
        REQUIRE_NOTHROW(dser->as_float("test_val", test_double));
        REQUIRE_EQ(test_double, doctest::Approx(112E-6).epsilon(0.0001));

        dser.emplace(test_obj3);

        REQUIRE_NOTHROW(dser->as_float("", test_double));
        REQUIRE(doctest::IsNaN(test_double));
    }

    TEST_CASE("as_int")
    {
        const nlohmann::json test_obj1 = nlohmann::json::parse("12345");

        std::optional<deserializer> dser{ test_obj1 };

        int test_int{};
        REQUIRE_NOTHROW(dser->as_int("", test_int));
        REQUIRE_EQ(test_int, 12'345);

        const nlohmann::json test_obj2 = nlohmann::json::parse(R"({"test_val": -2147483650})");
        dser.emplace(test_obj2);

        int64_t test_int2{};
        REQUIRE_NOTHROW(dser->as_int("test_val", test_int2));
        REQUIRE_EQ(test_int2, -2'147'483'650LL);
    }

    TEST_CASE("as_uint")
    {
        const nlohmann::json test_obj1 = nlohmann::json::parse("12345");

        std::optional<deserializer> dser{ test_obj1 };

        unsigned test_uint{};
        REQUIRE_NOTHROW(dser->as_uint("", test_uint));
        REQUIRE_EQ(test_uint, 12'345U);

        const nlohmann::json test_obj2 =
            nlohmann::json::parse(R"({"test_val": 9223372036854775810})");
        dser.emplace(test_obj2);

        uint64_t test_uint2{};
        REQUIRE_NOTHROW(dser->as_uint("test_val", test_uint2));
        REQUIRE_EQ(test_uint2, 9'223'372'036'854'775'810ULL);
    }

    TEST_CASE("as_enum")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_string")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_array")
    {
        SUBCASE("array of values")
        {
            // TODO: Implement test
        }

        SUBCASE("array of arrays")
        {
            // TODO: Implement test
        }

        SUBCASE("array of maps")
        {
            // TODO: Implement test
        }

        SUBCASE("array of tuples")
        {
            // TODO: Implement test
        }

        SUBCASE("array of optionals")
        {
            // TODO: Implement test
        }

        SUBCASE("array of variants")
        {
            // TODO: Implement test
        }

        SUBCASE("array of objects")
        {
            // TODO: Implement test
        }
    }

    TEST_CASE("as_map")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_multimap")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_tuple")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_optional")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_variant")
    {
        // TODO: Implement test
    }

    TEST_CASE("as_object")
    {
        const nlohmann::json test_obj1 = nlohmann::json::parse(
            R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": { "name": "Yolanda", "species": 2 }, "fruit_count": {"0": 2, "3": 4}})");

        std::optional<deserializer> dser{ test_obj1 };

        Person test_val1{};
        REQUIRE_NOTHROW(dser->as_object("", test_val1));
        REQUIRE_EQ(test_val1.age, 18);
        REQUIRE_EQ(test_val1.name, "Bill Garfield");
        REQUIRE_EQ(test_val1.friends.size(), 0);
        REQUIRE(test_val1.pet.has_value());
        REQUIRE_EQ(test_val1.pet->species, Pet::Species::Dog);
        REQUIRE_EQ(test_val1.fruit_count.size(), 2);
        REQUIRE_EQ(test_val1.fruit_count.at(Fruit::Apple), 2);

        const nlohmann::json test_obj2 = nlohmann::json::parse(
            R"({"test_val": {"age": 18, "name": "Bill Garfield", "friends": [], "pet": null, "fruit_count": {}}})");

        dser.emplace(test_obj2);

        REQUIRE_NOTHROW(dser->as_object("test_val", test_val1));
        REQUIRE_EQ(test_val1.age, 18);
        REQUIRE_FALSE(test_val1.pet.has_value());

        const nlohmann::json test_obj3 = nlohmann::json::parse(
            R"({"test_val": {"age": 18, "name": "Bill Garfield", "friends": [], "pet": [], "fruit_count": {}}})");

        dser.emplace(test_obj3);

        REQUIRE_THROWS_AS(dser->as_object("test_val", test_val1), deserialization_error);
    }
}
} //namespace extenser::tests
