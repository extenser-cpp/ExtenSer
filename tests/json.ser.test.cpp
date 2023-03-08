#include "json.test.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <magic_enum.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace extenser::tests
{
TEST_SUITE("json::serializer")
{
    using serializer = json_adapter::serializer_t;

    SCENARIO("a JSON serializer can be constructed properly without throwing")
    {
        GIVEN("an empty optional<serializer>")
        {
            std::optional<serializer> ser{};
            REQUIRE_FALSE(ser.has_value());

            WHEN("the serializer is (default) constructed")
            {
                REQUIRE_NOTHROW(ser.emplace());

                THEN("the serializer is constructed without exception")
                {
                    REQUIRE(ser.has_value());

                    AND_THEN("the serializer object is null and empty")
                    {
                        CHECK(ser->object().is_null());
                        CHECK(ser->object().empty());
                    }
                }
            }
        }
    }

    SCENARIO("a call to serializer::object() returns the underlying JSON without side effects")
    {
        GIVEN("a serializer containing a value")
        {
            static constexpr unsigned test_val = 22U;
            serializer ser{};
            ser.as_uint("", test_val);

            WHEN("object(lvalue) is called on the serializer")
            {
                const auto& obj = ser.object();

                THEN("the object is not empty")
                {
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object contains the correct value type")
                    {
                        CHECK(obj.is_number_unsigned());
                    }
                }
            }

            WHEN("object(rvalue) is called on the serializer")
            {
                const auto obj = std::move(ser).object();

                THEN("the object is not empty")
                {
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object contains the correct value type")
                    {
                        CHECK(obj.is_number_unsigned());
                    }
                    AND_THEN("the serializer's JSON member is in a valid, moved-from state")
                    {
                        CHECK(ser.object().is_null());
                        CHECK(ser.object().empty());
                    }
                }
            }
        }
    }

    SCENARIO("a boolean can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};

            WHEN("a bool is serialized")
            {
                REQUIRE_NOTHROW(ser.as_bool("", true));
                const auto& obj = ser.object();

                THEN("the JSON object holds a boolean")
                {
                    REQUIRE(obj.is_boolean());
                    CHECK(obj.get<bool>());
                }
            }

            WHEN("a bool is serialized as a sub-object")
            {
                REQUIRE_NOTHROW(ser.as_bool("test_val", false));
                const auto& obj = ser.object();

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds a boolean")
                    {
                        REQUIRE(sub_obj.is_boolean());
                        CHECK_FALSE(sub_obj.get<bool>());
                    }
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a float can be serialized to JSON", T_Float, float, double)
    {
        static constexpr double test_epsilon{ 0.0001 };

        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a float is serialized")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Float>::min() };

                REQUIRE_NOTHROW(ser.as_float("", test_val));

                THEN("the JSON object holds a float")
                {
                    REQUIRE(obj.is_number_float());
                    CHECK_EQ(obj.get<double>(),
                        doctest::Approx(static_cast<double>(test_val)).epsilon(test_epsilon));
                }
            }

            WHEN("NaN is serialized")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Float>::quiet_NaN() };

                REQUIRE_NOTHROW(ser.as_float("", test_val));

                THEN("the JSON object holds a NaN")
                {
                    REQUIRE(obj.is_number_float());
                    CHECK(doctest::IsNaN<double>(obj.get<double>()));
                }
            }

            WHEN("INF is serialized")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Float>::infinity() };

                REQUIRE_NOTHROW(ser.as_float("", test_val));

                THEN("the JSON object holds INF")
                {
                    REQUIRE(obj.is_number_float());
                    CHECK_EQ(obj.get<double>(), test_val);
                }
            }

            WHEN("a float is serialized as a sub-object")
            {
                static constexpr auto test_val = static_cast<T_Float>(3.141592653589793);

                REQUIRE_NOTHROW(ser.as_float("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds a float")
                    {
                        REQUIRE(sub_obj.is_number_float());
                        CHECK_EQ(sub_obj.get<double>(),
                            doctest::Approx(static_cast<double>(test_val)).epsilon(test_epsilon));
                    }
                }
            }
        }
    }

    SCENARIO_TEMPLATE(
        "a signed integer can be serialized to JSON", T_Int, int8_t, int16_t, int32_t, int64_t)
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a signed integer is serialized")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Int>::max() };

                REQUIRE_NOTHROW(ser.as_int("", test_val));

                THEN("the JSON object holds a signed integer")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK_FALSE(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<T_Int>(), test_val);
                }
            }

            WHEN("a signed integer is serialized as a sub-object")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Int>::min() };

                REQUIRE_NOTHROW(ser.as_int("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds a signed integer")
                    {
                        REQUIRE(sub_obj.is_number_integer());
                        CHECK_FALSE(sub_obj.is_number_unsigned());
                        CHECK_EQ(sub_obj.get<T_Int>(), test_val);
                    }
                }
            }
        }
    }

    SCENARIO_TEMPLATE("an unsigned integer can be serialized to JSON", T_Int, uint8_t, uint16_t,
        uint32_t, uint64_t)
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("an unsigned integer is serialized")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Int>::max() };

                REQUIRE_NOTHROW(ser.as_uint("", test_val));

                THEN("the JSON object holds an unsigned integer")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<T_Int>(), test_val);
                }
            }

            WHEN("an unsigned integer is serialized as a sub-object")
            {
                static constexpr auto test_val{ std::numeric_limits<T_Int>::min() };

                REQUIRE_NOTHROW(ser.as_uint("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds an unsigned integer")
                    {
                        REQUIRE(sub_obj.is_number_integer());
                        CHECK(sub_obj.is_number_unsigned());
                        CHECK_EQ(sub_obj.get<T_Int>(), test_val);
                    }
                }
            }
        }
    }

    SCENARIO("an enum can be serialized to JSON")
    {
        enum class TestCode : uint8_t
        {
            Code1 = 0x01U,
            CodeA = 0x0AU,
            CodeB = 0x0BU,
            CodeX = 0xFFU,
        };

        enum PlainEnum
        {
            VALUE_1,
            VALUE_2,
            VALUE_3,
            VALUE_XX = -1,
        };

        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a signed, scoped enum is serialized")
            {
                static constexpr Fruit test_val{ Fruit::Pineapple };

                REQUIRE_NOTHROW(ser.as_enum("", test_val));

#if defined(EXTENSER_USE_MAGIC_ENUM)
                THEN("the JSON object holds the enum name")
                {
                    REQUIRE(obj.is_string());
                    CHECK_EQ(obj.get<std::string>(), magic_enum::enum_name<Fruit>(test_val));
                }
#else
                THEN("the JSON object holds a signed integer")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK_FALSE(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<Fruit>(), test_val);
                }
#endif
            }

            WHEN("an unsigned, scoped enum is serialized")
            {
                static constexpr TestCode test_val{ TestCode::CodeB };

                REQUIRE_NOTHROW(ser.as_enum("", test_val));

#if defined(EXTENSER_USE_MAGIC_ENUM)
                THEN("the JSON object holds the enum name")
                {
                    REQUIRE(obj.is_string());
                    CHECK_EQ(obj.get<std::string>(), magic_enum::enum_name<TestCode>(test_val));
                }
#else
                THEN("the JSON object holds an unsigned integer")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<TestCode>(), test_val);
                }
#endif
            }

            WHEN("an out-of-range enum is serialized")
            {
                static constexpr TestCode test_val{ 0xCCU };

#if defined(EXTENSER_USE_MAGIC_ENUM)
                THEN("an exception is thrown")
                {
                    CHECK_THROWS_AS(ser.as_enum("", test_val), serialization_error);
                }
#else
                REQUIRE_NOTHROW(ser.as_enum("", test_val));

                THEN("the JSON still holds the underlying value")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<TestCode>(), test_val);
                }
#endif
            }

            WHEN("a plain enum is serialized")
            {
                static constexpr PlainEnum test_val{ VALUE_XX };

                REQUIRE_NOTHROW(ser.as_enum("", test_val));

#if defined(EXTENSER_USE_MAGIC_ENUM)
                THEN("the JSON object holds the enum name")
                {
                    REQUIRE(obj.is_string());
                    CHECK_EQ(obj.get<std::string>(), magic_enum::enum_name<PlainEnum>(test_val));
                }
#else
                THEN("the JSON object holds an integer")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK_EQ(obj.get<PlainEnum>(), test_val);
                }
#endif
            }

            WHEN("an enum is serialized as a sub-object")
            {
                static constexpr Fruit test_val{ Fruit::Pineapple };

                REQUIRE_NOTHROW(ser.as_enum("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

#if defined(EXTENSER_USE_MAGIC_ENUM)
                    AND_THEN("the sub-object holds the enum name")
                    {
                        REQUIRE(sub_obj.is_string());
                        CHECK_EQ(
                            sub_obj.get<std::string>(), magic_enum::enum_name<Fruit>(test_val));
                    }
#else
                    AND_THEN("the sub-object holds an integer")
                    {
                        REQUIRE(sub_obj.is_number_integer());
                        CHECK_EQ(sub_obj.get<Fruit>(), test_val);
                    }
#endif
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a string can be serialized to JSON", T_Str, const char*, const char[23],
        std::string_view, std::string)
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a string is serialized")
            {
                const T_Str test_val = "Mary had a little lamb";

                REQUIRE_NOTHROW(ser.as_string("", test_val));

                THEN("the JSON object holds a string")
                {
                    REQUIRE(obj.is_string());
                    CHECK_EQ(obj.get<std::string>(), test_val);
                }
            }
        }
    }

    // TODO: Refactor as BDD-style test
    TEST_CASE("as_array")
    {
        SUBCASE("array of types")
        {
            static constexpr std::array test_val1{ 1, 2, 5, 7, 9 };
            static constexpr std::string_view test_val2{ "Strings can be arrays too!" };
            const std::vector<std::string> test_val3{ "This", "is", "a", "test", "right?" };
            const std::string test_val4{ "Test test test" };
            const std::vector<bool> test_val5{ true, false, false, true, true, false, true, false,
                false, true, true, true, true, false, false, false };

            std::optional<serializer> ser{ std::in_place };
            const auto& obj = ser.value().object();

            CHECK_NOTHROW(ser->as_array("", test_val1));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_array());
            REQUIRE_EQ(obj.size(), test_val1.size());
            REQUIRE(obj[0].is_number_integer());
            REQUIRE_FALSE(obj[0].is_number_unsigned());
            REQUIRE(std::equal(obj.begin(), obj.end(), test_val1.begin()));

            ser.emplace();

            CHECK_NOTHROW(ser->as_array("test_val", test_val2));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("test_val"));

            const auto& sub_obj = obj.at("test_val");

            REQUIRE(sub_obj.is_array());
            REQUIRE_EQ(sub_obj.size(), test_val2.size());
            REQUIRE(sub_obj[0].is_number());
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val2.begin()));

            CHECK_NOTHROW(ser->as_array("test_val", test_val3));
            REQUIRE(sub_obj.is_array());
            REQUIRE_EQ(sub_obj.size(), test_val3.size());
            REQUIRE(sub_obj[0].is_string());
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val3.begin()));

            CHECK_NOTHROW(ser->as_array("test_val", test_val4));
            REQUIRE(sub_obj.is_array());
            REQUIRE_EQ(sub_obj.size(), test_val4.size());
            REQUIRE(sub_obj[0].is_number());
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val4.begin()));

            CHECK_NOTHROW(ser->as_array("test_val", test_val5));
            REQUIRE(sub_obj.is_array());
            REQUIRE_EQ(sub_obj.size(), test_val5.size());
            REQUIRE(sub_obj[0].is_boolean());
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val5.begin(),
                [](const nlohmann::json& json_obj, const bool bool_val)
                { return json_obj.get<bool>() == bool_val; }));
        }

        SUBCASE("array of arrays")
        {
            static constexpr std::array<std::array<int, 5>, 5> test_val1{ { { 1, 1, 1, 1, 1 },
                { 1, 2, 3, 4, 5 }, { 4, 6, 8, 9, 19 }, { -1, -3, 12, 13, 10 },
                { 0, 0, 0, 0, 0 } } };

            const auto test_val2 = create_3d_vec(5, 5, 5);

            std::optional<serializer> ser{ std::in_place };
            const auto& obj = ser.value().object();

            CHECK_NOTHROW(ser->as_array("", test_val1));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_array());
            REQUIRE_EQ(obj.size(), test_val1.size());
            REQUIRE(obj[0].is_array());
            REQUIRE_EQ(obj[0].size(), test_val1[0].size());
            REQUIRE(obj[0][0].is_number_integer());
            REQUIRE(std::equal(obj.begin(), obj.end(), test_val1.begin()));

            ser.emplace();

            CHECK_NOTHROW(ser->as_array("test_val", test_val2));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("test_val"));

            const auto& sub_obj = obj.at("test_val");

            REQUIRE(sub_obj.is_array());
            REQUIRE_EQ(sub_obj.size(), test_val2.size());
            REQUIRE(sub_obj[0].is_array());
            REQUIRE_EQ(sub_obj[0].size(), test_val2[0].size());
            REQUIRE(sub_obj[0][0].is_array());
            REQUIRE_EQ(sub_obj[0][0].size(), test_val2[0][0].size());
            REQUIRE(sub_obj[0][0][0].is_number_float());
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val2.begin()));
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
            const Person person1{ 10, "Timmy Johnson", {}, { Pet{ "Sparky", Pet::Species::Dog } },
                { { Fruit::Banana, 2 }, { Fruit::Apple, 2 } } };

            const Person person2{ 22, "Franky Johnson", { person1 },
                { Pet{ "Tommy", Pet::Species::Turtle } },
                { { Fruit::Apple, 1 }, { Fruit::Mango, 2 } } };

            const Person person3{ 44, "Bertha Jenkins", {}, {}, { { Fruit::Kiwi, 12 } } };

            const std::vector<Person> test_val1{ person1, person2, person3 };

            std::optional<serializer> ser{ std::in_place };
            const auto& obj = ser.value().object();

            CHECK_NOTHROW(ser->as_array("", test_val1));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_array());
            REQUIRE_EQ(obj.size(), test_val1.size());

            for (size_t i = 0; i < test_val1.size(); ++i)
            {
                REQUIRE_FALSE(obj[i].empty());
                REQUIRE(obj[i].is_object());
                REQUIRE(obj[i].contains("age"));
                REQUIRE(obj[i]["age"].is_number_integer());
                REQUIRE_EQ(obj[i]["age"].get<int>(), test_val1[i].age);
            }
        }
    }

    SCENARIO("a map can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a std::map is serialized")
            {
                const std::map<int, std::string> test_val{ { 33, "Benjamin Burton" },
                    { 99, "John Johnson" }, { 444, "Reed Carmichael" } };

                REQUIRE_NOTHROW(ser.as_map("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds all of its members")
                    {
                        CHECK_EQ(obj.size(), test_val.size());

                        for (const auto& [k, v] : test_val)
                        {
                            const auto key_str = std::to_string(k);

                            REQUIRE(obj.contains(key_str));
                            REQUIRE(obj[key_str].is_string());
                            CHECK_EQ(obj[key_str].get<std::string>(), v);
                        }
                    }
                }
            }

            WHEN("a std::unordered_map is serialized")
            {
                const std::unordered_map<std::string, Person> test_val{
                    { "Henrietta",
                        Person{ 16, "Henrietta Payne", {}, Pet{ "Ron", Pet::Species::Fish }, {} } },
                    { "Jerome", Person{ 12, "Jerome Banks", {}, {}, {} } },
                    { "Rachel", Person{ 22, "Rachel Franks", {}, {}, {} } },
                    { "Ricardo",
                        Person{
                            19, "Ricardo Montoya", {}, Pet{ "Sinbad", Pet::Species::Cat }, {} } }
                };

                REQUIRE_NOTHROW(ser.as_map("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds all of its members")
                    {
                        CHECK_EQ(obj.size(), test_val.size());

                        for (const auto& [k, v] : test_val)
                        {
                            REQUIRE(obj.contains(k));
                            REQUIRE(obj[k].is_object());
                            REQUIRE(obj[k].contains("age"));
                            REQUIRE(obj[k]["age"].is_number_integer());
                            CHECK_EQ(obj[k]["age"], v.age);
                        }
                    }
                }
            }

            WHEN("a std::map is serialized as a subobject")
            {
                const std::map<int, std::string> test_val{ { 33, "Benjamin Burton" },
                    { 99, "John Johnson" }, { 444, "Reed Carmichael" } };

                REQUIRE_NOTHROW(ser.as_map("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds an object")
                    {
                        REQUIRE(sub_obj.is_object());
                        REQUIRE_FALSE(sub_obj.empty());

                        AND_THEN("the object holds all of its members")
                        {
                            CHECK_EQ(sub_obj.size(), test_val.size());

                            for (const auto& [k, v] : test_val)
                            {
                                const auto key_str = std::to_string(k);

                                REQUIRE(sub_obj.contains(key_str));
                                REQUIRE(sub_obj[key_str].is_string());
                                CHECK_EQ(sub_obj[key_str].get<std::string>(), v);
                            }
                        }
                    }
                }
            }
        }
    }

    SCENARIO("a multimap can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a std::multimap is serialized")
            {
                const std::multimap<char, std::string> test_val{ { 'a', "Apple" },
                    { 'a', "Aardvark" }, { 'b', "Brush" }, { 'c', "Cleaver" }, { 'd', "Danger" },
                    { 'd', "Donut" } };

                REQUIRE_NOTHROW(ser.as_multimap("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds all of its members")
                    {
                        for (const auto& [k, v] : test_val)
                        {
                            const auto key_str = std::to_string(k);
                            REQUIRE(obj.contains(key_str));
                            const auto& val_obj = obj[key_str];

                            REQUIRE(val_obj.is_array());
                            const auto find_it = std::find(val_obj.cbegin(), val_obj.cend(), v);
                            CHECK_NE(find_it, val_obj.cend());
                        }
                    }
                }
            }

            WHEN("a std::unordered_multimap is serialized")
            {
                const std::unordered_multimap<std::string, std::string> test_val{
                    { "Stan Lee", "Marvel" }, { "Jack Kirby", "Marvel" }, { "Jack Kirby", "DC" },
                    { "Mike Mignola", "Dark Horse" }, { "Mike Mignola", "DC" },
                    { "Mike Mignola", "Marvel" }, { "Grant Morrison", "DC" }
                };

                REQUIRE_NOTHROW(ser.as_multimap("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds all of its members")
                    {
                        for (const auto& [k, v] : test_val)
                        {
                            REQUIRE(obj.contains(k));
                            const auto& val_obj = obj.at(k);

                            REQUIRE(val_obj.is_array());
                            const auto find_it = std::find(val_obj.cbegin(), val_obj.cend(), v);
                            CHECK_NE(find_it, val_obj.cend());
                        }
                    }
                }
            }

            WHEN("a std::multimap is serialized as a subobject")
            {
                const std::multimap<char, std::string> test_val{ { 'a', "Apple" },
                    { 'a', "Aardvark" }, { 'b', "Brush" }, { 'c', "Cleaver" }, { 'd', "Danger" },
                    { 'd', "Donut" } };

                REQUIRE_NOTHROW(ser.as_multimap("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds an object")
                    {
                        REQUIRE(sub_obj.is_object());
                        REQUIRE_FALSE(sub_obj.empty());

                        AND_THEN("the object holds all of its members")
                        {
                            for (const auto& [k, v] : test_val)
                            {
                                const auto key_str = std::to_string(k);
                                REQUIRE(sub_obj.contains(key_str));
                                const auto& val_obj = sub_obj[key_str];

                                REQUIRE(val_obj.is_array());
                                const auto find_it = std::find(val_obj.cbegin(), val_obj.cend(), v);
                                CHECK_NE(find_it, val_obj.cend());
                            }
                        }
                    }
                }
            }
        }
    }

    SCENARIO("a tuple can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a std::tuple is serialized")
            {
                const std::tuple<int, std::string, double> test_val{ 14, "Yellow Bus", 78.48 };

                REQUIRE_NOTHROW(ser.as_tuple("", test_val));

                THEN("the JSON object holds an array")
                {
                    REQUIRE(obj.is_array());

                    AND_THEN("the array holds all its members")
                    {
                        REQUIRE_EQ(obj.size(), std::tuple_size_v<decltype(test_val)>);

                        REQUIRE(obj[0].is_number_integer());
                        CHECK_EQ(obj[0].get<int>(), std::get<0>(test_val));

                        REQUIRE(obj[1].is_string());
                        CHECK_EQ(obj[1].get<std::string>(), std::get<1>(test_val));

                        REQUIRE(obj[2].is_number_float());
                        CHECK_EQ(obj[2].get<double>(),
                            doctest::Approx(std::get<2>(test_val)).epsilon(0.0001));
                    }
                }
            }

            WHEN("a std::pair is serialized")
            {
                const std::pair test_val{ Fruit::Orange, Pet{ "Valerie", Pet::Species::Bird } };

                REQUIRE_NOTHROW(ser.as_tuple("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds 'first' and 'second'")
                    {
                        REQUIRE(obj.contains("first"));
#if defined(EXTENSER_USE_MAGIC_ENUM)
                        REQUIRE(obj["first"].is_string());
                        CHECK_EQ(obj["first"].get<std::string>(),
                            magic_enum::enum_name<Fruit>(test_val.first));
#else
                        REQUIRE(obj["first"].is_number_integer());
                        CHECK_EQ(obj["first"].get<Fruit>(), test_val.first);
#endif

                        REQUIRE(obj.contains("second"));
                        REQUIRE(obj["second"].is_object());

                        REQUIRE(obj["second"].contains("name"));
                        REQUIRE(obj["second"]["name"].is_string());
                        CHECK_EQ(obj["second"]["name"].get<std::string>(), test_val.second.name);

                        REQUIRE(obj["second"].contains("species"));
#if defined(EXTENSER_USE_MAGIC_ENUM)
                        REQUIRE(obj["second"]["species"].is_string());
                        CHECK_EQ(obj["second"]["species"].get<std::string>(),
                            magic_enum::enum_name<Pet::Species>(test_val.second.species));
#else
                        REQUIRE(obj["second"]["species"].is_number_integer());
                        CHECK_EQ(
                            obj["second"]["species"].get<Pet::Species>(), test_val.second.species);
#endif
                    }
                }
            }

            WHEN("an empty std::tuple is serialized as a subobject")
            {
                static constexpr std::tuple<> test_val{};

                REQUIRE_NOTHROW(ser.as_tuple("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds an empty array")
                    {
                        CHECK(sub_obj.is_array());
                        CHECK(sub_obj.empty());
                    }
                }
            }
        }
    }

    SCENARIO("an optional can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("an optional with a value is serialized")
            {
                const std::optional<int> test_val{ 22 };

                REQUIRE_NOTHROW(ser.as_optional("", test_val));

                THEN("the JSON object holds the value type")
                {
                    REQUIRE(obj.is_number_integer());
                    CHECK_FALSE(obj.is_number_unsigned());
                    CHECK_EQ(obj.get<int>(), test_val.value());
                }
            }

            WHEN("an optional without a value is serialized")
            {
                const std::optional<Person> test_val{};

                REQUIRE_NOTHROW(ser.as_optional("", test_val));

                THEN("the JSON object holds null")
                {
                    CHECK(obj.is_null());
                }
            }

            WHEN("an optional with a value is serialized as a sub-object")
            {
                const std::optional<std::string> test_val{ "Hello, world!" };

                REQUIRE_NOTHROW(ser.as_optional("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds the value type")
                    {
                        REQUIRE(sub_obj.is_string());
                        CHECK_EQ(sub_obj.get<std::string>(), test_val.value());
                    }
                }
            }
        }
    }

    // TODO: Refactor as BDD-style test
    TEST_CASE("as_variant")
    {
        std::optional<serializer> ser{ std::in_place };
        const auto& obj = ser.value().object();

        SUBCASE("int-float-string")
        {
            using test_type1 = std::variant<int, float, std::string>;
            const test_type1 test_val1_1{ 22 };
            const test_type1 test_val1_2{ -87.111f };
            const test_type1 test_val1_3{ "Hello, world" };

            CHECK_NOTHROW(ser->as_variant("", test_val1_1));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("v_idx"));
            REQUIRE(obj["v_idx"].is_number_unsigned());
            REQUIRE_EQ(obj["v_idx"].get<size_t>(), test_val1_1.index());

            REQUIRE(obj.contains("v_val"));
            REQUIRE(obj["v_val"].is_number_integer());
            REQUIRE_EQ(obj["v_val"].get<int>(), std::get<int>(test_val1_1));

            CHECK_NOTHROW(ser->as_variant("", test_val1_2));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("v_idx"));
            REQUIRE(obj["v_idx"].is_number_unsigned());
            REQUIRE_EQ(obj["v_idx"].get<size_t>(), test_val1_2.index());

            REQUIRE(obj.contains("v_val"));
            REQUIRE(obj["v_val"].is_number_float());
            REQUIRE_EQ(obj["v_val"].get<float>(), std::get<float>(test_val1_2));

            CHECK_NOTHROW(ser->as_variant("", test_val1_3));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("v_idx"));
            REQUIRE(obj["v_idx"].is_number_unsigned());
            REQUIRE_EQ(obj["v_idx"].get<size_t>(), test_val1_3.index());

            REQUIRE(obj.contains("v_val"));
            REQUIRE(obj["v_val"].is_string());
            REQUIRE_EQ(obj["v_val"].get<std::string>(), std::get<std::string>(test_val1_3));
        }

        SUBCASE("monostate-pair-object")
        {
            using test_type2 = std::variant<std::monostate, std::pair<double, double>, Pet>;
            const test_type2 test_val2_1{};
            const test_type2 test_val2_2{ std::pair{ 1234.5678, -0.99999999 } };
            const test_type2 test_val2_3{ Pet{ "Barry", Pet::Species::Fish } };

            ser.emplace();

            CHECK_NOTHROW(ser->as_variant("test_val", test_val2_1));
            REQUIRE_FALSE(obj.empty());
            REQUIRE(obj.is_object());
            REQUIRE(obj.contains("test_val"));

            const auto& sub_obj = obj["test_val"];

            REQUIRE(sub_obj.is_object());
            REQUIRE(sub_obj.contains("v_idx"));

            const auto& sub_idx = sub_obj["v_idx"];

            REQUIRE(sub_idx.is_number_unsigned());
            REQUIRE_EQ(sub_idx.get<size_t>(), test_val2_1.index());

            REQUIRE(sub_obj.contains("v_val"));

            const auto& sub_val = sub_obj["v_val"];

            REQUIRE(sub_val.is_null());

            CHECK_NOTHROW(ser->as_variant("test_val", test_val2_2));

            REQUIRE(sub_obj.is_object());
            REQUIRE(sub_obj.contains("v_idx"));
            REQUIRE(sub_idx.is_number_unsigned());
            REQUIRE_EQ(sub_idx.get<size_t>(), test_val2_2.index());

            const auto& [test_1, test_2] = std::get<std::pair<double, double>>(test_val2_2);

            REQUIRE(sub_obj.contains("v_val"));
            REQUIRE(sub_val.is_object());
            REQUIRE(sub_val.contains("first"));
            REQUIRE(sub_val.contains("second"));
            REQUIRE(sub_val["first"].is_number_float());
            REQUIRE_EQ(sub_val["first"].get<double>(), test_1);
            REQUIRE(sub_val["second"].is_number_float());
            REQUIRE_EQ(sub_val["second"].get<double>(), test_2);

            CHECK_NOTHROW(ser->as_variant("test_val", test_val2_3));

            REQUIRE(sub_obj.is_object());
            REQUIRE(sub_obj.contains("v_idx"));
            REQUIRE(sub_idx.is_number_unsigned());
            REQUIRE_EQ(sub_idx.get<size_t>(), test_val2_3.index());

            REQUIRE(sub_obj.contains("v_val"));
            REQUIRE(sub_val.is_object());
            REQUIRE(sub_val.contains("name"));
            REQUIRE(sub_val["name"].is_string());
            REQUIRE_EQ(sub_val["name"].get<std::string>(), std::get<Pet>(test_val2_3).name);
        }

        SUBCASE("vector<enum>-vector<object>")
        {
            using test_type3 =
                std::variant<std::vector<int>, std::vector<std::variant<Fruit, Pet>>>;

            const test_type3 test_val3_1{ std::vector{ 1, 2, 7, 8, 9 } };
            const test_type3 test_val3_2{ std::vector<std::variant<Fruit, Pet>>{
                Fruit::Grape, Fruit::Orange, Pet{ "Sandra", Pet::Species::Snake } } };

            CHECK_NOTHROW(ser->as_variant("test_val", test_val3_1));

            const auto& sub_obj = obj["test_val"];

            REQUIRE(sub_obj.is_object());
            REQUIRE(sub_obj.contains("v_idx"));

            const auto& sub_idx = sub_obj["v_idx"];

            REQUIRE(sub_idx.is_number_unsigned());
            REQUIRE_EQ(sub_idx.get<size_t>(), test_val3_1.index());

            const auto& vec1 = std::get<std::vector<int>>(test_val3_1);

            REQUIRE(sub_obj.contains("v_val"));

            const auto& sub_val = sub_obj["v_val"];

            REQUIRE(sub_val.is_array());
            REQUIRE_EQ(sub_val.size(), vec1.size());
            REQUIRE(std::equal(sub_val.begin(), sub_val.end(), vec1.begin()));

            CHECK_NOTHROW(ser->as_variant("test_val", test_val3_2));

            REQUIRE(sub_obj.is_object());
            REQUIRE(sub_obj.contains("v_idx"));
            REQUIRE(sub_idx.is_number_unsigned());
            REQUIRE_EQ(sub_idx.get<size_t>(), test_val3_2.index());

            const auto& vec2 = std::get<std::vector<std::variant<Fruit, Pet>>>(test_val3_2);

            REQUIRE(sub_obj.contains("v_val"));
            REQUIRE(sub_val.is_array());
            REQUIRE_EQ(sub_val.size(), vec2.size());

            REQUIRE(sub_val[0].is_object());
            REQUIRE_EQ(sub_val[0]["v_idx"].get<size_t>(), vec2[0].index());
            REQUIRE(sub_val[1].is_object());
            REQUIRE_EQ(sub_val[1]["v_idx"].get<size_t>(), vec2[1].index());
            REQUIRE(sub_val[2].is_object());
            REQUIRE_EQ(sub_val[2]["v_idx"].get<size_t>(), vec2[2].index());

#if defined(EXTENSER_USE_MAGIC_ENUM)
            REQUIRE(sub_val[0]["v_val"].is_string());
            REQUIRE_EQ(sub_val[0]["v_val"].get<std::string>(),
                magic_enum::enum_name<Fruit>(std::get<Fruit>(vec2[0])));
            REQUIRE(sub_val[1]["v_val"].is_string());
            REQUIRE_EQ(sub_val[1]["v_val"].get<std::string>(),
                magic_enum::enum_name<Fruit>(std::get<Fruit>(vec2[1])));
#else
            REQUIRE(sub_val[0]["v_val"].is_number_integer());
            REQUIRE_EQ(sub_val[0]["v_val"].get<Fruit>(), std::get<Fruit>(vec2[0]));
            REQUIRE(sub_val[1]["v_val"].is_number_integer());
            REQUIRE_EQ(sub_val[1]["v_val"].get<Fruit>(), std::get<Fruit>(vec2[1]));
#endif

            REQUIRE(sub_val[2]["v_val"].is_object());
            REQUIRE(sub_val[2]["v_val"].contains("name"));
            REQUIRE(sub_val[2]["v_val"]["name"].is_string());
            REQUIRE_EQ(sub_val[2]["v_val"]["name"].get<std::string>(), std::get<Pet>(vec2[2]).name);
        }
    }

    SCENARIO("a user-defined class can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a user-defined class is serialized")
            {
                const Person test_val_friend{ 10, "Timmy Johnson", {},
                    { Pet{ "Sparky", Pet::Species::Dog } },
                    { { Fruit::Banana, 2 }, { Fruit::Apple, 2 } } };

                const Person test_val{ 22, "Franky Johnson", { test_val_friend },
                    { Pet{ "Tommy", Pet::Species::Turtle } },
                    { { Fruit::Apple, 1 }, { Fruit::Mango, 2 } } };

                REQUIRE_NOTHROW(ser.as_object("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object holds all of its members")
                    {
                        REQUIRE(obj.contains("age"));
                        REQUIRE(obj["age"].is_number_integer());
                        REQUIRE_FALSE(obj["age"].is_number_unsigned());
                        CHECK_EQ(obj["age"].get<int>(), test_val.age);

                        REQUIRE(obj.contains("name"));
                        REQUIRE(obj["name"].is_string());
                        CHECK_EQ(obj["name"].get<std::string>(), test_val.name);

                        REQUIRE(obj.contains("friends"));
                        REQUIRE(obj["friends"].is_array());
                        CHECK_EQ(obj["friends"].size(), test_val.friends.size());
                        CHECK_EQ(obj["friends"][0]["age"], test_val.friends[0].age);

                        REQUIRE(obj.contains("pet"));
                        REQUIRE(obj["pet"].is_object());
                        REQUIRE(obj["pet"].contains("name"));
                        REQUIRE(obj["pet"]["name"].is_string());
                        CHECK_EQ(obj["pet"]["name"].get<std::string>(), test_val.pet->name);
                        REQUIRE(obj["pet"].contains("species"));
#if defined(EXTENSER_USE_MAGIC_ENUM)
                        REQUIRE(obj["pet"]["species"].is_string());
                        CHECK_EQ(obj["pet"]["species"].get<std::string>(),
                            magic_enum::enum_name<Pet::Species>(test_val.pet->species));
#else
                        REQUIRE(obj["pet"]["species"].is_number_integer());
                        CHECK_EQ(obj["pet"]["species"].get<int>(),
                            static_cast<int>(test_val.pet->species));
#endif

                        REQUIRE(obj.contains("fruit_count"));
                        REQUIRE(obj["fruit_count"].is_object());
#if defined(EXTENSER_USE_MAGIC_ENUM)
                        REQUIRE(obj["fruit_count"].contains("Apple"));
                        REQUIRE(obj["fruit_count"]["Apple"].is_number_integer());
                        CHECK_EQ(
                            obj["fruit_count"]["Apple"], test_val.fruit_count.at(Fruit::Apple));
                        REQUIRE(obj["fruit_count"].contains("Mango"));
                        REQUIRE(obj["fruit_count"]["Mango"].is_number_integer());
                        CHECK_EQ(
                            obj["fruit_count"]["Mango"], test_val.fruit_count.at(Fruit::Mango));
#else
                        REQUIRE(obj["fruit_count"].contains("0"));
                        REQUIRE(obj["fruit_count"]["0"].is_number_integer());
                        CHECK_EQ(obj["fruit_count"]["0"], test_val.fruit_count.at(Fruit::Apple));
                        REQUIRE(obj["fruit_count"].contains("4"));
                        REQUIRE(obj["fruit_count"]["4"].is_number_integer());
                        CHECK_EQ(obj["fruit_count"]["4"], test_val.fruit_count.at(Fruit::Mango));
#endif
                    }
                }
            }

            WHEN("a user-defined class is serialized as a sub-object")
            {
                const Person test_val{ 44, "Bertha Jenkins", {}, {}, { { Fruit::Kiwi, 12 } } };

                REQUIRE_NOTHROW(ser.as_object("test_val", test_val));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds an object")
                    {
                        REQUIRE(sub_obj.is_object());
                        REQUIRE_FALSE(sub_obj.empty());

                        AND_THEN("the object holds all its members")
                        {
                            REQUIRE(sub_obj.contains("age"));
                            REQUIRE(sub_obj["age"].is_number_integer());
                            REQUIRE_FALSE(sub_obj["age"].is_number_unsigned());
                            CHECK_EQ(sub_obj["age"].get<int>(), test_val.age);

                            REQUIRE(sub_obj.contains("name"));
                            REQUIRE(sub_obj["name"].is_string());
                            CHECK_EQ(sub_obj["name"].get<std::string>(), test_val.name);

                            REQUIRE(sub_obj.contains("friends"));
                            REQUIRE(sub_obj["friends"].is_array());
                            CHECK(sub_obj["friends"].empty());

                            REQUIRE(sub_obj.contains("pet"));
                            CHECK(sub_obj["pet"].is_null());

                            REQUIRE(sub_obj.contains("fruit_count"));
                            REQUIRE(sub_obj["fruit_count"].is_object());
#if defined(EXTENSER_USE_MAGIC_ENUM)
                            CHECK_FALSE(sub_obj["fruit_count"].contains("Apple"));
                            REQUIRE(sub_obj["fruit_count"].contains("Kiwi"));
                            REQUIRE(sub_obj["fruit_count"]["Kiwi"].is_number_integer());
                            CHECK_EQ(sub_obj["fruit_count"]["Kiwi"],
                                test_val.fruit_count.at(Fruit::Kiwi));
#else
                            CHECK_FALSE(sub_obj["fruit_count"].contains("0"));
                            REQUIRE(sub_obj["fruit_count"].contains("3"));
                            REQUIRE(sub_obj["fruit_count"]["3"].is_number_integer());
                            CHECK_EQ(
                                sub_obj["fruit_count"]["3"], test_val.fruit_count.at(Fruit::Kiwi));
#endif
                        }
                    }
                }
            }
        }
    }

    SCENARIO("null types can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a null value is serialized")
            {
                REQUIRE_NOTHROW(ser.as_null(""));

                THEN("the JSON object holds null")
                {
                    CHECK(obj.is_null());
                }
            }

            WHEN("a null value is serialized as a sub-object")
            {
                REQUIRE_NOTHROW(ser.as_null("test_val"));

                THEN("the JSON object has a sub-object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE(obj.contains("test_val"));
                    const auto& sub_obj = obj["test_val"];

                    AND_THEN("the sub-object holds null")
                    {
                        CHECK(sub_obj.is_null());
                    }
                }
            }

            WHEN("a nullptr is serialized")
            {
                REQUIRE_NOTHROW(ser.as_object("", nullptr));

                THEN("the JSON object holds null")
                {
                    CHECK(obj.is_null());
                }
            }

            WHEN("a std::nullopt is serialized")
            {
                REQUIRE_NOTHROW(ser.as_object("", std::nullopt));

                THEN("the JSON object holds null")
                {
                    CHECK(obj.is_null());
                }
            }

            WHEN("a std::monostate is serialized")
            {
                REQUIRE_NOTHROW(ser.as_object("", std::monostate{}));

                THEN("the JSON object holds null")
                {
                    CHECK(obj.is_null());
                }
            }
        }
    }
}
} //namespace extenser::tests
