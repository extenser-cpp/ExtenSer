// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#define EXTENSER_ASSERT_THROW

//#define EXTENSER_BITSERY_EXACT_SZ

#include "test_helpers.hpp"
#include "extenser_bitsery.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

namespace extenser::tests
{
TEST_SUITE("bitsery adapter")
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

    // TEST_CASE("a call to serializer::object() throws when empty")
    // {
    //     CHECK_THROWS(std::ignore = serializer{}.object());
    // }

    TEST_CASE("a bitsery deserializer can be constructed properly without throwing")
    {
        const std::vector<std::uint8_t> obj{ 0x00U, 0x00U, 0x00U, 0x00U };
        std::optional<deserializer> dser{};
        REQUIRE_FALSE(dser.has_value());

        REQUIRE_NOTHROW(dser.emplace(obj));
        CHECK(dser.has_value());
    }

    // TEST_CASE("a bitsery deserializer constructed with an empty object fails its post-condition")
    // {
    //     const std::vector<std::uint8_t> obj{};
    //     std::optional<deserializer> dser{};

    //     CHECK_THROWS(dser.emplace(obj));
    // }

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
        std::array<int, 5>, std::string_view, std::vector<int>, std::deque<std::vector<double>>,
        std::list<Person>, std::forward_list<std::string>, std::set<int>,
        std::unordered_multiset<std::string>, span<Person>)
    {
        serializer ser{};

        const auto expected_val = create_test_val<T_Arr>();

        REQUIRE_NOTHROW(ser.as_array("", expected_val));

        deserializer dser{ ser.object() };

        T_Arr test_val{};
        dser.as_array("", test_val);

        if constexpr (containers::traits<T_Arr>::is_sequential)
        {
            CHECK(std::equal(std::begin(test_val), std::end(test_val), std::begin(expected_val)));
        }
        else
        {
            for (const auto& key : expected_val)
            {
                CHECK_EQ(test_val.count(key), expected_val.count(key));
            }
        }
    }

    TEST_CASE("a map-like container can be serialized to bitsery")
    {
        serializer ser{};

        SUBCASE("std::map")
        {
            const std::map<int, std::string> expected_val{ { 33, "Benjamin Burton" },
                { 99, "John Johnson" }, { 444, "Reed Carmichael" } };

            REQUIRE_NOTHROW(ser.as_map("", expected_val));

            deserializer dser{ ser.object() };

            std::map<int, std::string> test_val{};
            dser.as_map("", test_val);

            CHECK_EQ(test_val, expected_val);
        }

        SUBCASE("std::unordered_map")
        {
            const std::unordered_map<std::string, Person> expected_val{
                { "Henrietta",
                    Person{ 16, "Henrietta Payne", {}, Pet{ "Ron", Pet::Species::Fish }, {} } },
                { "Jerome", Person{ 12, "Jerome Banks", {}, {}, {} } },
                { "@Rachel", Person{ 22, "Rachel Franks", {}, {}, {} } },
                { "Ricardo",
                    Person{ 19, "Ricardo Montoya", {}, Pet{ "Sinbad", Pet::Species::Cat }, {} } }
            };

            REQUIRE_NOTHROW(ser.as_map("", expected_val));

            deserializer dser{ ser.object() };

            std::unordered_map<std::string, Person> test_val{};
            dser.as_map("", test_val);

            CHECK_EQ(test_val, expected_val);
        }
    }

    TEST_CASE("a multimap-like container can be serialized to bitsery")
    {
        serializer ser{};

        SUBCASE("std::multimap")
        {
            const std::multimap<char, std::string> expected_val{ { 'a', "Apple" },
                { 'a', "Aardvark" }, { 'b', "Brush" }, { 'c', "Cleaver" }, { 'd', "Danger" },
                { 'd', "Donut" } };

            REQUIRE_NOTHROW(ser.as_multimap("", expected_val));

            deserializer dser{ ser.object() };

            std::multimap<char, std::string> test_val{};
            dser.as_multimap("", test_val);

            for (const auto& [k, v] : expected_val)
            {
                CHECK_EQ(test_val.count(k), expected_val.count(k));
            }
        }

        SUBCASE("std::unordered_multimap")
        {
            const std::unordered_multimap<std::string, std::string> expected_val{
                { "Stan Lee", "Marvel" }, { "Jack Kirby", "Marvel" }, { "Jack Kirby", "DC" },
                { "Mike Mignola", "Dark Horse" }, { "Mike Mignola", "DC" },
                { "Mike Mignola", "Marvel" }, { "Grant Morrison", "DC" }
            };

            REQUIRE_NOTHROW(ser.as_multimap("", expected_val));

            deserializer dser{ ser.object() };

            std::unordered_multimap<std::string, std::string> test_val{};
            dser.as_multimap("", test_val);

            for (const auto& [k, v] : expected_val)
            {
                CHECK_EQ(test_val.count(k), expected_val.count(k));
            }
        }
    }

    TEST_CASE("a tuple can be serialized to bitsery")
    {
        serializer ser{};

        SUBCASE("tuple<int, string, double>")
        {
            const std::tuple<int, std::string, double> expected_val{ 14, "Yellow Bus", 78.48 };

            REQUIRE_NOTHROW(ser.as_tuple("", expected_val));

            deserializer dser{ ser.object() };

            std::tuple<int, std::string, double> test_val{};
            dser.as_tuple("", test_val);

            CHECK_EQ(std::get<0>(test_val), std::get<0>(expected_val));
            CHECK_EQ(std::get<1>(test_val), std::get<1>(expected_val));
            CHECK_EQ(std::get<2>(test_val), std::get<2>(expected_val));
        }

        SUBCASE("std::pair")
        {
            const std::pair expected_val{ Fruit::Orange, Pet{ "Valerie", Pet::Species::Bird } };

            REQUIRE_NOTHROW(ser.as_tuple("", expected_val));

            deserializer dser{ ser.object() };

            std::pair<Fruit, Pet> test_val{};
            dser.as_tuple("", test_val);

            CHECK_EQ(std::get<0>(test_val), std::get<0>(expected_val));
            CHECK_EQ(std::get<1>(test_val), std::get<1>(expected_val));
        }

        SUBCASE("empty tuple")
        {
            const std::tuple<> expected_val{};

            REQUIRE_NOTHROW(ser.as_tuple("", expected_val));

            const auto obj = std::move(ser).object();
            CHECK(obj.empty());
        }
    }

    TEST_CASE("an optional can be serialized to bitsery")
    {
        serializer ser{};

        SUBCASE("optional with a value")
        {
            const std::optional<int> expected_val{ 22 };

            REQUIRE_NOTHROW(ser.as_optional("", expected_val));

            deserializer dser{ ser.object() };

            std::optional<int> test_val{};
            dser.as_optional("", test_val);

            REQUIRE(test_val.has_value());
            CHECK_EQ(test_val.value(), expected_val.value());
        }

        SUBCASE("optional without a value")
        {
            const std::optional<Person> expected_val{};

            REQUIRE_NOTHROW(ser.as_optional("", expected_val));

            deserializer dser{ ser.object() };

            std::optional<Person> test_val{};
            dser.as_optional("", test_val);

            REQUIRE_FALSE(test_val.has_value());
        }
    }

    TEST_CASE("a variant can be serialized to bitsery")
    {
        using test_type = std::variant<std::monostate, int, double, std::string, Person>;
        static constexpr double test_epsilon{ 0.0001 };

        serializer ser{};

        SUBCASE("std::monostate")
        {
            const test_type expected_val{};

            REQUIRE_NOTHROW(ser.as_variant("", expected_val));

            deserializer dser{ ser.object() };

            test_type test_val{};
            dser.as_variant("", test_val);

            CHECK(std::holds_alternative<std::monostate>(test_val));
        }

        SUBCASE("int")
        {
            const test_type expected_val{ 22 };

            REQUIRE_NOTHROW(ser.as_variant("", expected_val));

            deserializer dser{ ser.object() };

            test_type test_val{};
            dser.as_variant("", test_val);

            REQUIRE(std::holds_alternative<int>(test_val));
            CHECK_EQ(std::get<int>(test_val), std::get<int>(expected_val));
        }

        SUBCASE("double")
        {
            const test_type expected_val{ -87.111 };

            REQUIRE_NOTHROW(ser.as_variant("", expected_val));

            deserializer dser{ ser.object() };

            test_type test_val{};
            dser.as_variant("", test_val);

            REQUIRE(std::holds_alternative<double>(test_val));
            CHECK_EQ(std::get<double>(test_val),
                doctest::Approx(std::get<double>(expected_val)).epsilon(test_epsilon));
        }

        SUBCASE("string")
        {
            const test_type expected_val{ "Hello, world" };

            REQUIRE_NOTHROW(ser.as_variant("", expected_val));

            deserializer dser{ ser.object() };

            test_type test_val{};
            dser.as_variant("", test_val);

            REQUIRE(std::holds_alternative<std::string>(test_val));
            CHECK_EQ(std::get<std::string>(test_val), std::get<std::string>(expected_val));
        }

        SUBCASE("object")
        {
            const test_type expected_val{ Person{ 55, "Earl Bixly", {}, {}, {} } };

            REQUIRE_NOTHROW(ser.as_variant("", expected_val));

            deserializer dser{ ser.object() };

            test_type test_val{};
            dser.as_variant("", test_val);

            REQUIRE(std::holds_alternative<Person>(test_val));
            CHECK_EQ(std::get<Person>(test_val), std::get<Person>(expected_val));
        }
    }

    TEST_CASE("a user-defined class can be serialized to bitsery")
    {
        serializer ser{};

        const Person expected_val_friend{ 10, "Timmy Johnson", {},
            { Pet{ "Sparky", Pet::Species::Dog } }, { { Fruit::Banana, 2 }, { Fruit::Apple, 2 } } };

        const Person expected_val{ 22, "Franky Johnson", { expected_val_friend },
            { Pet{ "Tommy", Pet::Species::Turtle } },
            { { Fruit::Apple, 1 }, { Fruit::Mango, 2 } } };

        REQUIRE_NOTHROW(ser.as_object("", expected_val));

        deserializer dser{ ser.object() };

        Person test_val{};
        dser.as_object("", test_val);

        REQUIRE_FALSE(test_val.friends.empty());
        CHECK_EQ(test_val, expected_val);
        CHECK_EQ(test_val.friends.front(), expected_val.friends.front());
    }

    TEST_CASE("null types can be serialized to bitsery")
    {
        serializer ser{};
        std::optional<deserializer> dser{};

        REQUIRE_NOTHROW(ser.as_null(""));
        const auto obj = std::move(ser).object();
        REQUIRE(obj.empty());

        REQUIRE_NOTHROW(ser.as_int("", 2));
        REQUIRE_NOTHROW(dser.emplace(ser.object()));

        REQUIRE_NOTHROW(dser->as_null(""));
    }

    TEST_CASE("a type with serialize as a member can be serialized to bitsery")
    {
        serializer ser{};

        static constexpr Bar expected_val(4);

        REQUIRE_NOTHROW(ser.as_object("", expected_val));

        deserializer dser{ ser.object() };

        Bar test_val{ 0 };
        dser.as_object("", test_val);

        CHECK_EQ(test_val, expected_val);
    }

    struct NoDefault
    {
        NoDefault() = delete;
        NoDefault(int num) : number(num) {}

        template<typename S>
        void serialize(extenser::generic_serializer<S>& ser)
        {
            ser.as_int("", number);
        }

        int number;
    };

    TEST_CASE("README Example")
    {
        extenser::easy_serializer<bitsery_adapter> ser{};

        // Serialize default constructible type
        const std::string input_str = "Hello, world!";
        ser.serialize_object(input_str);

        const auto output_str = ser.deserialize_object<std::string>();
        CHECK_EQ(output_str, input_str);

        // Serialize non-default constructible type
        NoDefault input_nd(2);
        ser.serialize_object(input_nd);

        NoDefault out_nd(1);
        ser.deserialize_object(out_nd);
        CHECK_EQ(out_nd.number, 2);
    }
}
} //namespace extenser::tests
