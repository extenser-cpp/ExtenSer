// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#define EXTENSER_ASSERT_THROW

#include "extenser_bitsery.hpp"
#include "test_helpers.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <limits>
#include <optional>
#include <vector>

namespace extenser::tests
{

using serializer = bitsery_adapter::serializer_t;
using deserializer = bitsery_adapter::deserializer_t;

TEST_CASE("a bitsery serializer can be constructed properly without throwing")
{
    std::optional<serializer> ser{};
    REQUIRE_FALSE(ser.has_value());

    REQUIRE_NOTHROW(ser.emplace());
    CHECK(ser.has_value());
}

TEST_CASE("a call to serializer::object() returns the underlying bytes without side effects")
{
    static constexpr unsigned test_val{ 22U };

    serializer ser{};
    ser.as_uint("", test_val);

    REQUIRE_NOTHROW(std::ignore = ser.object());

    const auto& obj = ser.object();
    REQUIRE_FALSE(obj.empty());

    const auto moved_obj = std::move(ser).object();

    CHECK_FALSE(moved_obj.empty());
    CHECK(obj.empty());
}

TEST_CASE("a call to serializer::object() throws when empty")
{
    CHECK_THROWS(std::ignore = serializer{}.object());
}

TEST_CASE("a bitsery deserializer can be constructed properly without throwing")
{
    const std::vector<std::uint8_t> obj{ 0x00U, 0x00U, 0x00U, 0x00U };
    std::optional<deserializer> dser{};
    REQUIRE_FALSE(dser.has_value());

    REQUIRE_NOTHROW(dser.emplace(obj));
    CHECK(dser.has_value());
}

TEST_CASE("a bitsery deserializer constructed with an empty object fails its post-condition")
{
    const std::vector<std::uint8_t> obj{};
    std::optional<deserializer> dser{};

    CHECK_THROWS(dser.emplace(obj));
}

TEST_CASE("a boolean can be serialized to bitsery")
{
    serializer ser{};

    REQUIRE_NOTHROW(ser.as_bool("", true));
    REQUIRE_NOTHROW(ser.as_bool("", false));

    deserializer dser{ ser.object() };

    bool test_val{};
    dser.as_bool("", test_val);

    CHECK(test_val);

    dser.as_bool("", test_val);
    CHECK_FALSE(test_val);
}

TEST_CASE_TEMPLATE("a float can be serialized to bitsery", T_Float, float, double)
{
    static constexpr double test_epsilon{ 0.0001 };
    serializer ser{};

    SUBCASE("a normal float can be serialzed")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Float>::min() };

        REQUIRE_NOTHROW(ser.as_float("", expected_val));

        deserializer dser{ ser.object() };

        T_Float test_val{};
        dser.as_float("", test_val);

        CHECK_EQ(
            test_val, doctest::Approx(static_cast<double>(expected_val)).epsilon(test_epsilon));
    }

    SUBCASE("NaN can be serialized")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Float>::quiet_NaN() };

        REQUIRE_NOTHROW(ser.as_float("", expected_val));

        deserializer dser{ ser.object() };

        T_Float test_val{};
        dser.as_float("", test_val);

        CHECK(doctest::IsNaN<T_Float>(test_val));
    }

    SUBCASE("INF can be serialized")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Float>::infinity() };

        REQUIRE_NOTHROW(ser.as_float("", expected_val));

        deserializer dser{ ser.object() };

        T_Float test_val{};
        dser.as_float("", test_val);

        CHECK_EQ(test_val, expected_val);
    }
}

TEST_CASE_TEMPLATE("a signed integer can be serialized to bitsery", T_Int, std::int8_t,
    std::int16_t, std::int32_t, std::int64_t)
{
    serializer ser{};

    SUBCASE("min value")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Int>::min() };

        REQUIRE_NOTHROW(ser.as_int("", expected_val));

        deserializer dser{ ser.object() };

        T_Int test_val{};
        dser.as_int("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    SUBCASE("max value")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Int>::max() };

        REQUIRE_NOTHROW(ser.as_int("", expected_val));

        deserializer dser{ ser.object() };

        T_Int test_val{};
        dser.as_int("", test_val);

        CHECK_EQ(test_val, expected_val);
    }
}

TEST_CASE_TEMPLATE("an unsigned integer can be serialized to bitsery", T_Int, std::uint8_t,
    std::uint16_t, std::uint32_t, std::uint64_t)
{
    serializer ser{};

    SUBCASE("min value")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Int>::min() };

        REQUIRE_NOTHROW(ser.as_uint("", expected_val));

        deserializer dser{ ser.object() };

        T_Int test_val{};
        dser.as_uint("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    SUBCASE("max value")
    {
        static constexpr auto expected_val{ std::numeric_limits<T_Int>::max() };

        REQUIRE_NOTHROW(ser.as_uint("", expected_val));

        deserializer dser{ ser.object() };

        T_Int test_val{};
        dser.as_uint("", test_val);

        CHECK_EQ(test_val, expected_val);
    }
}

TEST_CASE("an enum can be serialized to bitsery")
{
    serializer ser{};

    SUBCASE("signed, scoped enum")
    {
        static constexpr Fruit expected_val{ Fruit::Pineapple };

        REQUIRE_NOTHROW(ser.as_enum("", expected_val));

        deserializer dser{ ser.object() };

        Fruit test_val{};
        dser.as_enum("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    SUBCASE("unsigned, scoped enum")
    {
        static constexpr TestCode expected_val{ TestCode::CodeB };

        REQUIRE_NOTHROW(ser.as_enum("", expected_val));

        deserializer dser{ ser.object() };

        TestCode test_val{};
        dser.as_enum("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    SUBCASE("plain enum")
    {
        static constexpr PlainEnum expected_val{ VALUE_XX };

        REQUIRE_NOTHROW(ser.as_enum("", expected_val));

        deserializer dser{ ser.object() };

        PlainEnum test_val{};
        dser.as_enum("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    SUBCASE("out-of-range enum")
    {
        static constexpr TestCode expected_val{ 0xCCU };

        REQUIRE_NOTHROW(ser.as_enum("", expected_val));

        deserializer dser{ ser.object() };

        TestCode test_val{};
        dser.as_enum("", test_val);

        CHECK_EQ(test_val, expected_val);
    }
}

TEST_CASE_TEMPLATE(
    "a string can be serialized to bitsery", T_Str, const char*, const char[23], std::string)
{
    serializer ser{};

    const T_Str expected_val = "Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE_TEMPLATE("a wide string can be serialized to bitsery", T_Str, const wchar_t*,
    const wchar_t[23], std::wstring)
{
    serializer ser{};

    const T_Str expected_val = L"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::wstring test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE_TEMPLATE("a UTF-16 string can be serialized to bitsery", T_Str, const char16_t*,
    const char16_t[23], std::u16string)
{
    serializer ser{};

    const T_Str expected_val = u"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u16string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE_TEMPLATE("a UTF-32 string can be serialized to bitsery", T_Str, const char32_t*,
    const char32_t[23], std::u32string)
{
    serializer ser{};

    const T_Str expected_val = U"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u32string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

#if defined(__cpp_char8_t)
TEST_CASE_TEMPLATE("a UTF-8 string can be serialized to bitsery", T_Str, const char8_t*,
    const char8_t[23], std::u8string)
{
    serializer ser{};

    const T_Str expected_val = u8"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u8string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}
#endif

TEST_CASE("a string_view can be serialized to bitsery")
{
    serializer ser{};

    const std::string_view expected_val = "Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE("a wstring_view can be serialized to bitsery")
{
    serializer ser{};

    const std::wstring_view expected_val = L"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::wstring test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE("a u16string_view can be serialized to bitsery")
{
    serializer ser{};

    const std::u16string_view expected_val = u"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u16string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

TEST_CASE("a u32string_view can be serialized to bitsery")
{
    serializer ser{};

    const std::u32string_view expected_val = U"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u32string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}

#if defined(__cpp_char_8_t)
TEST_CASE("a u8string_view can be serialized to bitsery")
{
    serializer ser{};

    const std::u8string_view expected_val = u8"Mary had a little lamb";

    REQUIRE_NOTHROW(ser.as_string("", expected_val));

    deserializer dser{ ser.object() };

    std::u8string test_val{};
    dser.as_string("", test_val);

    CHECK_EQ(test_val, expected_val);
}
#endif

TEST_CASE_TEMPLATE("an array-like container can be serialized to bitsery", T_Arr,
    std::array<int, 5>, std::string_view, std::vector<bool>, std::deque<std::vector<double>>,
    std::list<Person>, std::forward_list<std::string>, std::set<int>,
    std::unordered_multiset<std::string>, span<Person>)
{
    // TODO: Get test working

    // serializer ser{};

    // const auto expected_val = create_test_val<T_Arr>();

    // REQUIRE_NOTHROW(ser.as_array("", expected_val));

    // deserializer dser{ ser.object() };

    // T_Arr test_val{};
    // dser.as_array("", test_val);
    // CHECK(std::equal(std::begin(test_val), std::end(test_val), std::begin(expected_val)));
}

TEST_CASE("a map-like container can be serialized to bitsery") {}

TEST_CASE("a multimap-like container can be serialized to bitsery") {}

TEST_CASE("a tuple can be serialized to bitsery") {}

TEST_CASE("an optional can be serialized to bitsery") {}

TEST_CASE("a variant can be serialized to bitsery") {}

TEST_CASE("a user-defined class can be serialized to bitsery") {}

TEST_CASE("null types can be serialized to bitsery")
{
    serializer ser{};

    REQUIRE_NOTHROW(ser.as_null(""));
    REQUIRE_NOTHROW(ser.as_int("", 2));

    deserializer dser{ ser.object() };

    REQUIRE_NOTHROW(dser.as_null(""));
}
} //namespace extenser::tests
