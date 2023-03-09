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

    SCENARIO("a JSON deserializer can be constructed properly without throwing")
    {
        GIVEN("an empty JSON object and empty optional<deserializer>")
        {
            const nlohmann::json obj{};
            std::optional<deserializer> dser{};
            REQUIRE_FALSE(dser.has_value());

            WHEN("the deserializer is constructed with the JSON object")
            {
                REQUIRE_NOTHROW(dser.emplace(obj));

                THEN("the deserializer is constructed without exception")
                {
                    REQUIRE(dser.has_value());
                }
            }
        }
    }

    SCENARIO("a boolean can be deserialized from JSON")
    {
        GIVEN("a deserializer with a JSON value containing a boolean")
        {
            const nlohmann::json test_obj = false;
            deserializer dser{ test_obj };

            WHEN("a bool is deserialized")
            {
                bool test_val{ true };

                REQUIRE_NOTHROW(dser.as_bool("", test_val));

                THEN("the bool is properly assigned")
                {
                    CHECK_FALSE(test_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a boolean")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = true;
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                bool test_val{ false };

                REQUIRE_NOTHROW(dser.as_bool("test_val", test_val));

                THEN("the bool is properly assigned")
                {
                    CHECK(test_val);
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a float can be deserialized from JSON", T_Float, float, double)
    {
        static constexpr double test_epsilon{ 0.0001 };

        GIVEN("a deserializer with a JSON value containing a float")
        {
            static constexpr auto expected_val = static_cast<T_Float>(1.256);

            const nlohmann::json test_obj = expected_val;
            deserializer dser{ test_obj };

            WHEN("a float is deserialized")
            {
                T_Float test_val{};

                REQUIRE_NOTHROW(dser.as_float("", test_val));

                THEN("the float is properly assigned")
                {
                    CHECK_EQ(test_val,
                        doctest::Approx(static_cast<double>(expected_val)).epsilon(test_epsilon));
                }
            }
        }

        GIVEN("a deserializer with a JSON value containing NaN")
        {
            const auto test_obj = std::numeric_limits<T_Float>::quiet_NaN();
            deserializer dser{ test_obj };

            WHEN("a float is deserialized")
            {
                T_Float test_val{};

                REQUIRE_NOTHROW(dser.as_float("", test_val));

                THEN("the float is properly assigned")
                {
                    CHECK(doctest::IsNaN<T_Float>(test_val));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a float")
        {
            static constexpr auto expected_val = static_cast<T_Float>(112E-6);

            const auto test_obj = nlohmann::json::parse(R"({"test_val": 112E-6})");
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Float test_val{};

                REQUIRE_NOTHROW(dser.as_float("test_val", test_val));

                THEN("the float is properly assigned")
                {
                    CHECK_EQ(test_val,
                        doctest::Approx(static_cast<double>(expected_val)).epsilon(test_epsilon));
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "a signed integer can be deserialized from JSON", T_Int, int8_t, int16_t, int32_t, int64_t)
    {
        GIVEN("a deserializer with a JSON value containing a signed integer")
        {
            static constexpr auto expected_val = (std::numeric_limits<T_Int>::max)();

            const nlohmann::json test_obj = expected_val;
            deserializer dser{ test_obj };

            WHEN("the integer is deserialized")
            {
                T_Int test_val{};

                REQUIRE_NOTHROW(dser.as_int("", test_val));

                THEN("the integer is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a signed integer")
        {
            static constexpr auto expected_val = (std::numeric_limits<T_Int>::min)();

            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Int test_val{};

                REQUIRE_NOTHROW(dser.as_int("test_val", test_val));

                THEN("the integer is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }
    }

    SCENARIO_TEMPLATE("an unsigned integer can be deserialized from JSON", T_Int, uint8_t, uint16_t,
        uint32_t, uint64_t)
    {
        GIVEN("a deserializer with a JSON value containing an unsigned integer")
        {
            static constexpr auto expected_val = (std::numeric_limits<T_Int>::max)();

            const nlohmann::json test_obj = expected_val;
            deserializer dser{ test_obj };

            WHEN("the integer is deserialized")
            {
                T_Int test_val{};

                REQUIRE_NOTHROW(dser.as_uint("", test_val));

                THEN("the integer is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a signed integer")
        {
            static constexpr auto expected_val = (std::numeric_limits<T_Int>::min)();

            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Int test_val{};

                REQUIRE_NOTHROW(dser.as_uint("test_val", test_val));

                THEN("the integer is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }
    }

    SCENARIO("an enum can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    // TODO: Support more types (string_view should be nop, mutable containers should std::copy, vector is push_back'ed)
    SCENARIO_TEMPLATE("a string can be deserialized from JSON", T_Str, std::string)
    {
        GIVEN("a deserializer with a JSON value respresenting a string")
        {
            static constexpr std::string_view expected_val = "Mary had a little lamb";

            const nlohmann::json test_obj = expected_val;
            deserializer dser{ test_obj };

            WHEN("the string is deserialized")
            {
                T_Str test_val{};

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the string is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a string")
        {
            static constexpr std::string_view expected_val =
                "Hello from a really quite lengthy string";

            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the string is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "an array-like container can be deserialized from JSON", T_Arr, std::vector<bool>)
    {
        // TODO: Implement test
    }

    SCENARIO("a map-like container can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    SCENARIO("a multimap-like container can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    SCENARIO("a tuple can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    SCENARIO("an optional can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    SCENARIO("a variant can be deserialized from JSON")
    {
        // TODO: Implement test
    }

    SCENARIO("a user-defined class can be deserialized from JSON")
    {
        GIVEN("a deserializer with a JSON object representing a class")
        {
#if defined(EXTENSER_USE_MAGIC_ENUM)
            const nlohmann::json test_obj = nlohmann::json::parse(
                R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": { "name": "Yolanda", "species": "Dog" }, "fruit_count": {"Apple": 2, "Kiwi": 4}})");
#else
            const nlohmann::json test_obj = nlohmann::json::parse(
                R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": { "name": "Yolanda", "species": 2 }, "fruit_count": {"0": 2, "3": 4}})");
#endif

            deserializer dser{ test_obj };

            WHEN("the class is deserialized")
            {
                Person test_val{};

                REQUIRE_NOTHROW(dser.as_object("", test_val));

                THEN("the class is properly assigned")
                {
                    CHECK_EQ(test_val.age, 18);
                    CHECK_EQ(test_val.name, "Bill Garfield");
                    CHECK_EQ(test_val.friends.size(), 0);
                    REQUIRE(test_val.pet.has_value());
                    CHECK_EQ(test_val.pet->species, Pet::Species::Dog);
                    CHECK_EQ(test_val.fruit_count.size(), 2);
                    CHECK_EQ(test_val.fruit_count.at(Fruit::Apple), 2);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a sub-object representing a class")
        {
            const nlohmann::json test_obj = nlohmann::json::parse(
                R"({"test_val": {"age": 18, "name": "Bill Garfield", "friends": [], "pet": null, "fruit_count": {}}})");
            deserializer dser{ test_obj };

            WHEN("the sub-object is deserialized")
            {
                Person test_val{};

                REQUIRE_NOTHROW(dser.as_object("test_val", test_val));

                THEN("the class is properly assigned")
                {
                    CHECK_EQ(test_val.age, 18);
                    CHECK_FALSE(test_val.pet.has_value());
                }
            }
        }

        GIVEN("a deserializer with a JSON object that does not correctly represent a class")
        {
            const nlohmann::json test_obj = nlohmann::json::parse(
                R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": [], "fruit_count": {}})");
            deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                Person test_val{};

                THEN("a deserialization_error is thrown")
                {
                    REQUIRE_THROWS_AS(dser.as_object("", test_val), deserialization_error);
                }
            }
        }
    }
}
} //namespace extenser::tests
