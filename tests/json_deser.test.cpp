#include "json.test.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

TEST_SUITE("json::deserializer")
{
    using deserializer = extenser::json_adapter::deserializer_t;

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

        std::optional<deserializer> dser{};
        dser.emplace(test_obj1);

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

        std::optional<deserializer> dser{};
        dser.emplace(test_obj1);

        float test_float{};
        REQUIRE_NOTHROW(dser->as_float("", test_float));
        REQUIRE_EQ(test_float, doctest::Approx(1.256f).epsilon(0.0001));

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
        // TODO: Implement test
    }

    TEST_CASE("as_uint")
    {
        // TODO: Implement test
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
        // TODO: Implement test
    }
}