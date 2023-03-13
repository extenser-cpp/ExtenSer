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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

            WHEN("a float is deserialized")
            {
                T_Float test_val{};

                REQUIRE_NOTHROW(dser.as_float("", test_val));

                THEN("the float is properly assigned")
                {
                    CHECK_EQ(static_cast<double>(test_val),
                        doctest::Approx(static_cast<double>(expected_val)).epsilon(test_epsilon));
                }
            }
        }

        GIVEN("a deserializer with a JSON value containing NaN")
        {
            const nlohmann::json test_obj = std::numeric_limits<T_Float>::quiet_NaN();
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Float test_val{};

                REQUIRE_NOTHROW(dser.as_float("test_val", test_val));

                THEN("the float is properly assigned")
                {
                    CHECK_EQ(static_cast<double>(test_val),
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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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

    SCENARIO_TEMPLATE("an array-like container can be deserialized from JSON", T_Arr,
        std::vector<int>, std::list<int>, std::deque<int>, std::forward_list<int>, std::set<int>,
        std::multiset<int>, std::array<int, 5>, span<int>)
    {
        GIVEN("a deserializer with a JSON array")
        {
            const auto test_obj = nlohmann::json::parse("[1, 2, 3, 4, 5]");
            const deserializer dser{ test_obj };

            WHEN("the array is deserialized")
            {
                T_Arr test_val{};

                if constexpr (std::is_same_v<T_Arr, span<int>>)
                {
                    static std::array<int, 5> tmp_arr{};
                    span<int> tmp_span = tmp_arr;
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_array("", test_val));

                THEN("the array is properly assigned")
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        CHECK_EQ(*std::next(std::begin(test_val), i), i + 1);
                    }
                }
            }
        }

        GIVEN("a deserializer with an empty JSON array")
        {
            const auto test_obj = nlohmann::json::parse("[]");
            const deserializer dser{ test_obj };

            WHEN("the array is deserialized")
            {
                T_Arr test_val{};

                REQUIRE_NOTHROW(dser.as_array("", test_val));

                THEN("the deserialized array is empty")
                {
                    if constexpr (std::is_same_v<T_Arr, std::array<int, 5>>)
                    {
                        CHECK(std::all_of(std::begin(test_val), std::end(test_val),
                            [](const int n) { return n == 0; }));
                    }
                    else
                    {
                        CHECK(std::empty(test_val));
                    }
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing an array")
        {
            const auto test_obj = nlohmann::json::parse(R"({"test_val": [0, 1, 2, 3, 4]})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Arr test_val{};

                if constexpr (std::is_same_v<T_Arr, span<int>>)
                {
                    static std::array<int, 5> tmp_arr{};
                    span<int> tmp_span = tmp_arr;
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_array("test_val", test_val));

                THEN("the array is properly assigned")
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        CHECK_EQ(*std::next(std::begin(test_val), i), i);
                    }
                }
            }
        }
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
        GIVEN("a deserializer with a JSON array representing a tuple")
        {
            const std::tuple<int, double, std::string> expected_val{ 874, 9941.5523, "Germany" };

            const auto test_obj = nlohmann::json::parse(R"([874, 9941.5523, "Germany"])");
            const deserializer dser{ test_obj };

            WHEN("the array is deserialized")
            {
                std::tuple<int, double, std::string> test_val{};

                REQUIRE_NOTHROW(dser.as_tuple("", test_val));

                THEN("the tuple is properly assigned")
                {
                    CHECK_EQ(std::get<0>(test_val), std::get<0>(expected_val));
                    CHECK_EQ(std::get<1>(test_val),
                        doctest::Approx(std::get<1>(expected_val)).epsilon(0.0001));
                    CHECK_EQ(std::get<2>(test_val), std::get<2>(expected_val));
                }
            }
        }

        GIVEN("a deserializer with a JSON array representing a pair")
        {
            static constexpr std::pair<Fruit, int> expected_val{ Fruit::Grape, 45 };

            const auto test_obj = nlohmann::json::parse(R"([2, 45])");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                std::pair<Fruit, int> test_val{};

                REQUIRE_NOTHROW(dser.as_tuple("", test_val));

                THEN("the pair is properly assigned")
                {
                    CHECK_EQ(test_val.first, expected_val.first);
                    CHECK_EQ(test_val.second, expected_val.second);
                }
            }
        }
    }

    SCENARIO("an optional can be deserialized from JSON")
    {
        GIVEN("a deserializer with a JSON value representing an empty optional")
        {
            const nlohmann::json test_obj = nullptr;
            const deserializer dser{ test_obj };

            WHEN("the value is deserialized")
            {
                std::optional<Person> test_val{};

                REQUIRE_NOTHROW(dser.as_optional("", test_val));

                THEN("the optional is assigned as nullopt")
                {
                    CHECK_FALSE(test_val.has_value());
                }
            }
        }

        GIVEN("a deserializer with a JSON value representing an (non-empty) optional")
        {
            const auto test_obj = nlohmann::json::parse(
                R"({ "age": 33, "name": "Angela Barnes", "pet": null, "friends": [], "fruit_count": {} })");
            const deserializer dser{ test_obj };

            WHEN("the value is deserialized")
            {
                std::optional<Person> test_val{};

                REQUIRE_NOTHROW(dser.as_optional("", test_val));

                THEN("the optional is assigned a value")
                {
                    REQUIRE(test_val.has_value());
                    CHECK_EQ(test_val->age, 33);
                    CHECK_EQ(test_val->name, "Angela Barnes");
                    CHECK_FALSE(test_val->pet.has_value());
                    CHECK(test_val->fruit_count.empty());
                }
            }
        }
    }

    SCENARIO("a variant can be deserialized from JSON")
    {
        using test_type = std::variant<std::monostate, int, double, std::string, Person>;

        GIVEN("a deserializer with a JSON object representing a variant (monostate)")
        {
            const auto test_obj = nlohmann::json::parse(R"({"v_idx": 0, "v_val": null})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                test_type test_val{};

                REQUIRE_NOTHROW(dser.as_variant("", test_val));

                THEN("the variant is assigned as a monostate")
                {
                    REQUIRE(std::holds_alternative<std::monostate>(test_val));
                }
            }
        }

        GIVEN("a deserializer with a JSON object representing a variant (int)")
        {
            const auto test_obj = nlohmann::json::parse(R"({"v_idx": 1, "v_val": -8481})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                test_type test_val{};

                REQUIRE_NOTHROW(dser.as_variant("", test_val));

                THEN("the variant is assigned as an int")
                {
                    REQUIRE(std::holds_alternative<int>(test_val));
                    CHECK_EQ(std::get<int>(test_val), -8481);
                }
            }
        }

        GIVEN("a deserializer with a JSON object representing a variant (double)")
        {
            const auto test_obj = nlohmann::json::parse(R"({"v_idx": 2, "v_val": 566421.532})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                test_type test_val{};

                REQUIRE_NOTHROW(dser.as_variant("", test_val));

                THEN("the variant is assigned as a double")
                {
                    REQUIRE(std::holds_alternative<double>(test_val));
                    CHECK_EQ(std::get<double>(test_val), 566421.532);
                }
            }
        }

        GIVEN("a deserializer with a JSON object representing a variant (int)")
        {
            const auto test_obj =
                nlohmann::json::parse(R"({"v_idx": 3, "v_val": "Variants are great!"})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                test_type test_val{};

                REQUIRE_NOTHROW(dser.as_variant("", test_val));

                THEN("the variant is assigned as a string")
                {
                    REQUIRE(std::holds_alternative<std::string>(test_val));
                    CHECK_EQ(std::get<std::string>(test_val), "Variants are great!");
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a sub-object representing a variant "
              "(Person)")
        {
#if defined(EXTENSER_USE_MAGIC_ENUM)
            const auto test_obj = nlohmann::json::parse(
                R"({"test_val": {"v_idx": 4, "v_val": {"age": 91, "name": "Gretl Hansel", "pet": {"name": "Fritz", "species": "Cat"}, "friends": [], "fruit_count": {}}}})");
#else
            const auto test_obj = nlohmann::json::parse(
                R"({"test_val": {"v_idx": 4, "v_val": {"age": 91, "name": "Gretl Hansel", "pet": {"name": "Fritz", "species": 1}, "friends": [], "fruit_count": {}}}})");
#endif
            const deserializer dser{ test_obj };

            WHEN("the sub-object is deserialized")
            {
                test_type test_val{};

                REQUIRE_NOTHROW(dser.as_variant("test_val", test_val));

                THEN("the variant is assigned as a class")
                {
                    REQUIRE(std::holds_alternative<Person>(test_val));

                    const auto& [age, name, friends, pet, fruit_count] = std::get<Person>(test_val);
                    CHECK_EQ(age, 91);
                    CHECK_EQ(name, "Gretl Hansel");
                    CHECK(pet.has_value());
                    CHECK(friends.empty());
                    CHECK(fruit_count.empty());
                }
            }
        }
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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
            const deserializer dser{ test_obj };

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
