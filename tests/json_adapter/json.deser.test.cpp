// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#include "../json.test.hpp"

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
#if defined(EXTENSER_USE_MAGIC_ENUM)
TEST_SUITE("json::deserializer (magic_enum)")
#else
TEST_SUITE("json::deserializer")
#endif
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

                    AND_WHEN("an attempt to deserialize occurs")
                    {
                        int test_val{};

                        THEN("a deserialization_error is thrown")
                        {
                            CHECK_THROWS_AS(dser->as_int("", test_val), deserialization_error);
                        }
                    }
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

        GIVEN("a deserializer with a JSON object NOT containing a boolean")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = 0;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                bool test_val{ false };

                THEN("a deserialization_error is thrown")
                {
                    CHECK_THROWS_AS(dser.as_bool("test_val", test_val), deserialization_error);
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
        GIVEN("a deserializer with a JSON value representing an enum")
        {
            static constexpr auto expected_val{ Fruit::Orange };

#if defined(EXTENSER_USE_MAGIC_ENUM)
            const nlohmann::json test_obj = "Orange";
#else
            const nlohmann::json test_obj = 5;
#endif
            const deserializer dser{ test_obj };

            WHEN("the enum is deserialized")
            {
                Fruit test_val{};

                REQUIRE_NOTHROW(dser.as_enum("", test_val));

                THEN("the enum is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing an enum")
        {
            static constexpr auto expected_val{ PlainEnum::VALUE_2 };

#if defined(EXTENSER_USE_MAGIC_ENUM)
            const nlohmann::json test_obj = "VALUE_2";
#else
            const nlohmann::json test_obj = 1;
#endif
            const deserializer dser{ test_obj };

            WHEN("the enum is deserialized")
            {
                PlainEnum test_val{};

                REQUIRE_NOTHROW(dser.as_enum("", test_val));

                THEN("the enum is properly assigned")
                {
                    CHECK_EQ(test_val, expected_val);
                }
            }
        }

        GIVEN("a deserializer with a JSON value representing an out-of-range enum")
        {
#if defined(EXTENSER_USE_MAGIC_ENUM)
            const nlohmann::json test_obj = "CodeC";
#else
            const nlohmann::json test_obj = 0x0CU;
#endif
            const deserializer dser{ test_obj };

            WHEN("the enum is deserialized")
            {
                TestCode test_val{};

#if defined(EXTENSER_USE_MAGIC_ENUM)
                THEN("an exception is thrown")
                {
                    CHECK_THROWS_AS(dser.as_enum("", test_val), deserialization_error);
                }
#else
                REQUIRE_NOTHROW(dser.as_enum("", test_val));

                THEN("the enum holds an invalid value")
                {
                    CHECK_EQ(static_cast<unsigned>(test_val), 0x0CU);
                }
#endif
            }
        }
    }

    SCENARIO_TEMPLATE("a string (or string-like container) can be deserialized from JSON", T_Str,
        std::array<char, 22>, std::string, std::vector<char>, span<char>)
    {
        static constexpr std::string_view expected_val = "Mary had a little lamb";

        GIVEN("a deserializer with a JSON value representing a string")
        {
            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the string is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char>>)
                {
                    static std::string tmp_val = "??????????????????????";
                    const span<char> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a string")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char>>)
                {
                    static std::string tmp_val = "??????????????????????";
                    const span<char> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a wide string (or string-like container) can be deserialized from JSON",
        T_Str, std::array<wchar_t, 22>, std::wstring, std::vector<wchar_t>, span<wchar_t>)
    {
        static constexpr std::wstring_view expected_val = L"Mary had a little lamb";

        GIVEN("a deserializer with a JSON value representing a wide string")
        {
            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the wide string is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<wchar_t>>)
                {
                    static std::wstring tmp_val = L"??????????????????????";
                    const span<wchar_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a wide string")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<wchar_t>>)
                {
                    static std::wstring tmp_val = L"??????????????????????";
                    const span<wchar_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the wide string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a UTF-16 string (or string-like container) can be deserialized from JSON",
        T_Str, std::array<char16_t, 22>, std::u16string, std::vector<char16_t>, span<char16_t>)
    {
        static constexpr std::u16string_view expected_val = u"Mary had a little lamb";

        GIVEN("a deserializer with a JSON value representing a UTF-16 string")
        {
            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the UTF-16 string is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char16_t>>)
                {
                    static std::u16string tmp_val = u"??????????????????????";
                    const span<char16_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the UTF-16 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a UTF-16 string")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char16_t>>)
                {
                    static std::u16string tmp_val = u"??????????????????????";
                    const span<char16_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the UTF-16 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }
    }

    SCENARIO_TEMPLATE("a UTF-32 string (or string-like container) can be deserialized from JSON",
        T_Str, std::array<char32_t, 22>, std::u32string, std::vector<char32_t>, span<char32_t>)
    {
        static constexpr std::u32string_view expected_val = U"Mary had a little lamb";

        GIVEN("a deserializer with a JSON value representing a UTF-32 string")
        {
            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the UTF-32 string is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char32_t>>)
                {
                    static std::u32string tmp_val = U"??????????????????????";
                    const span<char32_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the UTF-32 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a UTF-32 string")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char32_t>>)
                {
                    static std::u32string tmp_val = U"??????????????????????";
                    const span<char32_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the UTF-32 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }
    }

#if defined(__cpp_char8_t)
    SCENARIO_TEMPLATE("a UTF-8 string (or string-like container) can be deserialized from JSON",
        T_Str, std::array<char8_t, 22>, std::u8string, std::vector<char8_t>, span<char8_t>)
    {
        static constexpr std::u8string_view expected_val = u8"Mary had a little lamb";

        GIVEN("a deserializer with a JSON value representing a UTF-8 string")
        {
            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the UTF-8 string is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char8_t>>)
                {
                    static std::u8string tmp_val = u8"??????????????????????";
                    const span<char8_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the UTF-8 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing a UTF-8 string")
        {
            nlohmann::json test_obj;
            test_obj["test_val"] = expected_val;
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                T_Str test_val{};

                if constexpr (std::is_same_v<T_Str, span<char8_t>>)
                {
                    static std::u8string tmp_val = u8"??????????????????????";
                    const span<char8_t> tmp_span{ tmp_val.begin(), tmp_val.end() };
                    test_val = tmp_span;
                }

                REQUIRE_NOTHROW(dser.as_string("test_val", test_val));

                THEN("the UTF-8 string is properly assigned")
                {
                    CHECK(
                        std::equal(expected_val.begin(), expected_val.end(), std::begin(test_val)));
                }
            }
        }
    }
#endif

#if defined(__cpp_char8_t)
    SCENARIO_TEMPLATE(
        "a string_view (or other immutable container) is NOT changed by deserialization", T_Str,
        std::string_view, std::wstring_view, std::u16string_view, std::u32string_view,
        std::u8string_view, span<const char>)
#else
    SCENARIO_TEMPLATE(
        "a string_view (or other immutable container) is NOT changed by deserialization", T_Str,
        std::string_view, std::wstring_view, std::u16string_view, std::u32string_view,
        span<const char>)
#endif
    {
        GIVEN("a deserializer with a JSON value representing a string")
        {
            static constexpr auto expected_val = []
            {
                if constexpr (std::is_same_v<typename T_Str::value_type, wchar_t>)
                {
                    return std::wstring_view{ L"This string won't be seen" };
                }
                else if constexpr (std::is_same_v<typename T_Str::value_type, char16_t>)
                {
                    return std::u16string_view{ u"This string won't be seen" };
                }
                else if constexpr (std::is_same_v<typename T_Str::value_type, char32_t>)
                {
                    return std::u32string_view{ U"This string won't be seen" };
                }
#if defined(__cpp_char8_t)
                else if constexpr (std::is_same_v<typename T_Str::value_type, char8_t>)
                {
                    return std::u8string_view{ u8"This string won't be seen" };
                }
#endif
                else
                {
                    return std::string_view{ "This string won't be seen" };
                }
            }();

            const nlohmann::json test_obj = expected_val;
            const deserializer dser{ test_obj };

            WHEN("deserialize is called on the string_view")
            {
                T_Str test_val{};

                REQUIRE_NOTHROW(dser.as_string("", test_val));

                THEN("the string_view is unchanged")
                {
                    CHECK(test_val.empty());
                }
            }
        }
    }

    SCENARIO_TEMPLATE("an array-like container can be deserialized from JSON", T_Arr,
        std::vector<int>, std::list<int>, std::deque<int>, std::forward_list<int>,
        std::array<int, 5>, span<int>, std::set<int>, std::multiset<int>, std::unordered_set<int>,
        std::unordered_multiset<int>)
    {
        GIVEN("a deserializer with a JSON array")
        {
            static constexpr std::array expected_val{ 1, 5, 3, 4, 2 };

            const auto test_obj = nlohmann::json::parse("[1, 5, 3, 4, 2]");
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
                    REQUIRE_EQ(containers::adapter<T_Arr>::size(test_val), std::size(expected_val));

                    if constexpr (containers::traits<T_Arr>::is_sequential)
                    {
                        CHECK(std::equal(
                            std::begin(test_val), std::end(test_val), std::begin(expected_val)));
                    }
                    else
                    {
                        CHECK(std::is_permutation(
                            std::begin(test_val), std::end(test_val), std::begin(expected_val)));
                    }
                }
            }
        }

        if constexpr (not std::is_same_v<T_Arr, std::array<int, 5>>)
        {
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
                        CHECK(std::empty(test_val));
                    }
                }
            }
        }

        GIVEN("a deserializer with a JSON object containing an array")
        {
            static constexpr std::array expected_val{ 0, 4, 2, 3, 2 };
            const auto test_obj = nlohmann::json::parse(R"({"test_val": [0, 4, 2, 3, 2]})");
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
                    if constexpr (containers::traits<T_Arr>::is_sequential)
                    {
                        CHECK(std::equal(
                            std::begin(test_val), std::end(test_val), std::begin(expected_val)));
                    }
                    else
                    {
                        CHECK(std::is_permutation(
                            std::begin(test_val), std::end(test_val), std::begin(expected_val)));
                    }
                }
            }
        }
    }

    SCENARIO("a map-like container can be deserialized from JSON")
    {
        GIVEN("a deserializer with a JSON object representing a map")
        {
            const std::map<int, std::string> expected_val{ { 33, "Benjamin Burton" },
                { 99, "John Johnson" }, { 444, "Reed Carmichael" } };

            const auto test_obj = nlohmann::json::parse(
                R"({"@33": "Benjamin Burton", "@99": "John Johnson", "@444": "Reed Carmichael"})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                std::map<int, std::string> test_val{};

                REQUIRE_NOTHROW(dser.as_map("", test_val));

                THEN("the map is properly assigned")
                {
                    for (const auto& [k, v] : test_val)
                    {
                        CHECK_EQ(v, expected_val.at(k));
                    }
                }
            }
        }

        GIVEN("a deserializer with a JSON object representing an unordered_map")
        {
            const std::unordered_map<std::string, Person> expected_val{
                { "Henrietta",
                    Person{ 16, "Henrietta Payne", {}, Pet{ "Ron", Pet::Species::Fish }, {} } },
                { "Jerome", Person{ 12, "Jerome Banks", {}, {}, {} } },
                { "@Rachel", Person{ 22, "Rachel Franks", {}, {}, {} } },
                { "Ricardo",
                    Person{ 19, "Ricardo Montoya", {}, Pet{ "Sinbad", Pet::Species::Cat }, {} } }
            };

#if defined(EXTENSER_USE_MAGIC_ENUM)
            const auto test_obj = nlohmann::json::parse(R"({
"Henrietta": {"age": 16, "name": "Henrietta Payne", "friends": [], "pet": {"name": "Ron", "species": "Fish"}, "fruit_count": {}},
"Jerome": {"age": 12, "name": "Jerome Banks", "friends": [], "pet": null, "fruit_count": {}},
"@@Rachel": {"age": 22, "name": "Rachel Franks", "friends": [], "pet": null, "fruit_count": {}},
"Ricardo": {"age": 19, "name": "Ricardo Montoya", "friends": [], "pet": {"name": "Sinbad", "species": "Cat"}, "fruit_count": {}}
})");
#else
            const auto test_obj = nlohmann::json::parse(R"({
"Henrietta": {"age": 16, "name": "Henrietta Payne", "friends": [], "pet": {"name": "Ron", "species": 3}, "fruit_count": {}},
"Jerome": {"age": 12, "name": "Jerome Banks", "friends": [], "pet": null, "fruit_count": {}},
"@@Rachel": {"age": 22, "name": "Rachel Franks", "friends": [], "pet": null, "fruit_count": {}},
"Ricardo": {"age": 19, "name": "Ricardo Montoya", "friends": [], "pet": {"name": "Sinbad", "species": 1}, "fruit_count": {}}
})");
#endif
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                std::unordered_map<std::string, Person> test_val{};

                REQUIRE_NOTHROW(dser.as_map("", test_val));

                THEN("the unordered_map is properly assigned")
                {
                    for (const auto& [k, v] : test_val)
                    {
                        CHECK_EQ(v, expected_val.at(k));
                    }
                }
            }
        }
    }

    SCENARIO("a multimap-like container can be deserialized from JSON")
    {
        GIVEN("a deserializer with a JSON object representing a multimap")
        {
            const std::multimap<char, std::string> expected_val{ { 'a', "Apple" },
                { 'a', "Aardvark" }, { 'b', "Brush" }, { 'c', "Cleaver" }, { 'd', "Danger" },
                { 'd', "Donut" } };

            const auto test_obj = nlohmann::json::parse(R"({
"@97": ["Apple", "Aardvark"],
"@98": ["Brush"],
"@99": ["Cleaver"],
"@100": ["Danger", "Donut"]
})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                std::multimap<char, std::string> test_val{};

                REQUIRE_NOTHROW(dser.as_multimap("", test_val));

                THEN("the multimap is properly assigned")
                {
                    for (const auto& pair : test_val)
                    {
                        const auto find_it =
                            std::find(expected_val.cbegin(), expected_val.cend(), pair);
                        CHECK_NE(find_it, expected_val.cend());
                    }
                }
            }
        }

        GIVEN("a deserializer with a JSON object representing an unordered_multimap")
        {
            const std::unordered_multimap<std::string, std::string> expected_val{
                { "Stan Lee", "Marvel" }, { "Jack Kirby", "Marvel" }, { "Jack Kirby", "DC" },
                { "Mike Mignola", "Dark Horse" }, { "Mike Mignola", "DC" },
                { "Mike Mignola", "Marvel" }, { "Grant Morrison", "DC" }
            };

            const auto test_obj = nlohmann::json::parse(R"({
"Stan Lee": ["Marvel"],
"Jack Kirby": ["Marvel", "DC"],
"Mike Mignola": ["Dark Horse", "DC", "Marvel"],
"Grant Morrison": ["DC"]
})");
            const deserializer dser{ test_obj };

            WHEN("the object is deserialized")
            {
                std::unordered_map<std::string, std::string> test_val{};

                REQUIRE_NOTHROW(dser.as_multimap("", test_val));

                THEN("the unordered_multimap is properly assigned")
                {
                    for (const auto& pair : test_val)
                    {
                        const auto find_it =
                            std::find(expected_val.cbegin(), expected_val.cend(), pair);
                        CHECK_NE(find_it, expected_val.cend());
                    }
                }
            }
        }
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

#if defined(EXTENSER_USE_MAGIC_ENUM)
            const auto test_obj = nlohmann::json::parse(R"(["Grape", 45])");
#else
            const auto test_obj = nlohmann::json::parse(R"([2, 45])");
#endif
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
            const auto test_obj = nlohmann::json::parse(
                R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": { "name": "Yolanda", "species": "Dog" }, "fruit_count": {"Apple": 2, "Kiwi": 4}})");
#else
            const auto test_obj = nlohmann::json::parse(
                R"({"age": 18, "name": "Bill Garfield", "friends": [], "pet": { "name": "Yolanda", "species": 2 }, "fruit_count": {"@0": 2, "@3": 4}})");
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
            const auto test_obj = nlohmann::json::parse(
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
