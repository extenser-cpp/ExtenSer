#include "json.test.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <forward_list>
#include <limits>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_set>
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
                const auto& obj = ser.object();
                const auto moved_obj = std::move(ser).object();

                THEN("the object is not empty")
                {
                    REQUIRE_FALSE(moved_obj.empty());

                    AND_THEN("the object contains the correct value type")
                    {
                        CHECK(moved_obj.is_number_unsigned());
                    }
                    AND_THEN("the serializer's JSON member is in a valid, moved-from state")
                    {
                        CHECK(obj.is_null());
                        CHECK(obj.empty());
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

    SCENARIO_TEMPLATE("an array-like container can be serialized to JSON", T_Arr,
        std::array<int, 5>, std::string_view, std::vector<bool>, std::deque<std::vector<double>>,
        std::list<Person>, std::forward_list<std::string>, std::set<int>,
        std::unordered_multiset<std::string>, span<Person>)
    {
        GIVEN("a default-init serializer")
        {
            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a container is serialized")
            {
                const auto test_val = create_test_val<T_Arr>();

                REQUIRE_NOTHROW(ser.as_array("", test_val));

                THEN("the JSON object holds an array")
                {
                    REQUIRE(obj.is_array());

                    const auto calc_size = std::distance(std::begin(test_val), std::end(test_val));
                    CHECK_EQ(obj.size(), calc_size);
                }
            }
        }
    }

    SCENARIO("a map-like container can be serialized to JSON")
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

    SCENARIO("a multimap-like container can be serialized to JSON")
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

    SCENARIO("a variant can be serialized to JSON")
    {
        GIVEN("a default-init serializer")
        {
            using test_type = std::variant<std::monostate, int, double, std::string, Person>;

            serializer ser{};
            const auto& obj = ser.object();

            WHEN("a std::variant is serialized as a monostate (empty)")
            {
                const test_type test_val{};

                REQUIRE_NOTHROW(ser.as_variant("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object represents a variant holding null")
                    {
                        REQUIRE(obj.contains("v_idx"));
                        REQUIRE(obj["v_idx"].is_number_unsigned());
                        CHECK_EQ(obj["v_idx"].get<size_t>(), test_val.index());

                        REQUIRE(obj.contains("v_val"));
                        REQUIRE(obj["v_val"].is_null());
                    }
                }
            }

            WHEN("a std::variant is serialized as an integer")
            {
                const test_type test_val{ 22 };

                REQUIRE_NOTHROW(ser.as_variant("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object represents a variant holding an integer")
                    {
                        REQUIRE(obj.contains("v_idx"));
                        REQUIRE(obj["v_idx"].is_number_unsigned());
                        CHECK_EQ(obj["v_idx"].get<size_t>(), test_val.index());

                        REQUIRE(obj.contains("v_val"));
                        REQUIRE(obj["v_val"].is_number_integer());
                        CHECK_EQ(obj["v_val"].get<int>(), std::get<int>(test_val));
                    }
                }
            }

            WHEN("a std::variant is serialized as a float")
            {
                const test_type test_val{ -87.111 };

                REQUIRE_NOTHROW(ser.as_variant("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object represents a variant holding a float")
                    {
                        REQUIRE(obj.contains("v_idx"));
                        REQUIRE(obj["v_idx"].is_number_unsigned());
                        CHECK_EQ(obj["v_idx"].get<size_t>(), test_val.index());

                        REQUIRE(obj.contains("v_val"));
                        REQUIRE(obj["v_val"].is_number_float());
                        CHECK_EQ(obj["v_val"].get<double>(), std::get<double>(test_val));
                    }
                }
            }

            WHEN("a std::variant is serialized as a string")
            {
                const test_type test_val{ "Hello, world" };

                REQUIRE_NOTHROW(ser.as_variant("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object represents a variant holding a string")
                    {
                        REQUIRE(obj.contains("v_idx"));
                        REQUIRE(obj["v_idx"].is_number_unsigned());
                        CHECK_EQ(obj["v_idx"].get<size_t>(), test_val.index());

                        REQUIRE(obj.contains("v_val"));
                        REQUIRE(obj["v_val"].is_string());
                        CHECK_EQ(obj["v_val"].get<std::string>(), std::get<std::string>(test_val));
                    }
                }
            }

            WHEN("a std::variant is serialized as an object")
            {
                const test_type test_val{ Person{ 55, "Earl Bixly", {}, {}, {} } };

                REQUIRE_NOTHROW(ser.as_variant("", test_val));

                THEN("the JSON object holds an object")
                {
                    REQUIRE(obj.is_object());
                    REQUIRE_FALSE(obj.empty());

                    AND_THEN("the object represents a variant holding an object")
                    {
                        REQUIRE(obj.contains("v_idx"));
                        REQUIRE(obj["v_idx"].is_number_unsigned());
                        CHECK_EQ(obj["v_idx"].get<size_t>(), test_val.index());

                        REQUIRE(obj.contains("v_val"));
                        const auto& sub_val = obj["v_val"];
                        REQUIRE(sub_val.is_object());

                        REQUIRE(sub_val.contains("age"));
                        REQUIRE(sub_val["age"].is_number_integer());
                        CHECK_EQ(sub_val["age"].get<int>(), std::get<Person>(test_val).age);

                        REQUIRE(sub_val.contains("name"));
                        REQUIRE(sub_val["name"].is_string());
                        CHECK_EQ(
                            sub_val["name"].get<std::string>(), std::get<Person>(test_val).name);
                    }
                }
            }
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
