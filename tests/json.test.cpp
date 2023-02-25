#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

TEST_SUITE("json::serializer")
{
    using serializer = extenser::json_adapter::serializer_t;

    TEST_CASE("CTOR")
    {
        std::optional<serializer> ser{};
        REQUIRE_FALSE(ser.has_value());

        REQUIRE_NOTHROW(ser.emplace());
        REQUIRE(ser.has_value());

        const auto& obj = ser->object();

        REQUIRE(obj.is_null());
        REQUIRE(obj.empty());
    }

    TEST_CASE("object")
    {
        serializer ser{};
        ser.as_uint("", 22U);

        const auto& obj = ser.object();

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE(obj.is_number_unsigned());

        auto obj2 = std::move(ser).object();

        REQUIRE_FALSE(obj2.empty());
        REQUIRE(obj2.is_number());
        REQUIRE(obj2.is_number_integer());
        REQUIRE(obj2.is_number_unsigned());

        // Verify json member is in a moved-from state
        REQUIRE(obj.is_null());
        REQUIRE(obj.empty());
    }

    TEST_CASE("as_bool")
    {
        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        REQUIRE_NOTHROW(ser->as_bool("", true));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_boolean());
        REQUIRE(static_cast<bool>(obj));

        ser.reset();
        ser.emplace();

        REQUIRE_NOTHROW(ser->as_bool("test_val", false));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.is_structured());
        REQUIRE(obj.contains("test_val"));
        REQUIRE_FALSE(static_cast<bool>(obj.at("test_val")));

        REQUIRE_NOTHROW(ser->as_bool("test_val", 22));

        REQUIRE(static_cast<bool>(obj.at("test_val")));

        REQUIRE_NOTHROW(ser->as_bool("test_val", nullptr));

        REQUIRE_FALSE(static_cast<bool>(obj.at("test_val")));
    }

    TEST_CASE("as_float")
    {
        static constexpr double test_epsilon{ 0.0001 };

        static constexpr float test_val1{ std::numeric_limits<float>::min() };
        static constexpr double test_val2{ NAN };
        static constexpr double test_val3{ M_PI };
        static constexpr int64_t test_val4_i{ 1'234'567LL };
        static constexpr double test_val4{ test_val4_i };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        REQUIRE_NOTHROW(ser->as_float("", test_val1));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_float());
        REQUIRE_EQ(obj.get<float>(), doctest::Approx(test_val1).epsilon(test_epsilon));

        ser.reset();
        ser.emplace();

        REQUIRE_NOTHROW(ser->as_float("test_val", test_val2));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.is_structured());
        REQUIRE(obj.contains("test_val"));
        REQUIRE(obj.at("test_val").is_number_float());
        REQUIRE(doctest::IsNaN(obj.at("test_val").get<double>()));

        REQUIRE_NOTHROW(ser->as_float("test_val", test_val3));

        REQUIRE(obj.at("test_val").is_number_float());
        REQUIRE_EQ(obj.at("test_val").get<long double>(),
            doctest::Approx(test_val3).epsilon(test_epsilon));

        REQUIRE_NOTHROW(ser->as_float("test_val", test_val4_i));

        REQUIRE(obj.at("test_val").is_number_float());
        REQUIRE_EQ(
            obj.at("test_val").get<double>(), doctest::Approx(test_val4).epsilon(test_epsilon));
    }

    TEST_CASE("as_int")
    {
        static constexpr int test_val1{ 12'345 };
        static constexpr intmax_t test_val2{ std::numeric_limits<intmax_t>::min() };
        static constexpr int8_t test_val3{ 0x4A };
        static constexpr double test_val4_f{ 12.34 };
        static constexpr int16_t test_val4{ static_cast<int16_t>(test_val4_f) };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        REQUIRE_NOTHROW(ser->as_int("", test_val1));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE_FALSE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<int>(), test_val1);

        ser.reset();
        ser.emplace();

        REQUIRE_NOTHROW(ser->as_int("test_val", test_val2));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.is_structured());
        REQUIRE(obj.contains("test_val"));
        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE_FALSE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<intmax_t>(), test_val2);

        REQUIRE_NOTHROW(ser->as_int("test_val", test_val3));

        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE_FALSE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<int8_t>(), test_val3);

        REQUIRE_NOTHROW(ser->as_int("test_val", test_val4_f));

        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE_FALSE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<int16_t>(), test_val4);
    }

    TEST_CASE("as_uint")
    {
        static constexpr unsigned test_val1{ 12'345 };
        static constexpr uintmax_t test_val2{ std::numeric_limits<uintmax_t>::max() };
        static constexpr uint8_t test_val3{ 0x4A };
        static constexpr double test_val4_f{ 12.34 };
        static constexpr uint16_t test_val4{ static_cast<uint16_t>(test_val4_f) };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        REQUIRE_NOTHROW(ser->as_uint("", test_val1));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<unsigned>(), test_val1);

        ser.reset();
        ser.emplace();

        REQUIRE_NOTHROW(ser->as_uint("test_val", test_val2));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.is_structured());
        REQUIRE(obj.contains("test_val"));
        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<uintmax_t>(), test_val2);

        REQUIRE_NOTHROW(ser->as_uint("test_val", test_val3));

        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<uint8_t>(), test_val3);

        REQUIRE_NOTHROW(ser->as_uint("test_val", test_val4_f));

        REQUIRE(obj.at("test_val").is_number_integer());
        REQUIRE(obj.at("test_val").is_number_unsigned());
        REQUIRE_EQ(obj.at("test_val").get<uint16_t>(), test_val4);
    }

    TEST_CASE("as_string")
    {
        static constexpr std::string_view test_val1{ "Hello, world!" };
        static constexpr const char* test_val2{ "Mary had a little lamb" };
        static constexpr char test_val3[]{ "Whose fleece was white as snow" };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        REQUIRE_NOTHROW(ser->as_string("", test_val1));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_string());
        REQUIRE_EQ(obj.get<std::string>(), test_val1);

        ser.reset();
        ser.emplace();

        REQUIRE_NOTHROW(ser->as_string("test_val", test_val2));

        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.is_structured());
        REQUIRE(obj.contains("test_val"));
        REQUIRE(obj.at("test_val").is_string());
        REQUIRE_EQ(obj.at("test_val").get<std::string>(), test_val2);

        REQUIRE_NOTHROW(ser->as_string("test_val", test_val3));

        REQUIRE(obj.at("test_val").is_string());
        REQUIRE_EQ(obj.at("test_val").get<std::string>(), test_val3);

        const std::string test_val4{ "Goodbye, cruel world!" };
        REQUIRE_NOTHROW(ser->as_string("test_val", test_val4));

        REQUIRE(obj.at("test_val").is_string());
        REQUIRE_EQ(obj.at("test_val").get<std::string>(), test_val4);

        static constexpr std::array<char, 6> test_val5{ 'Q', 'W', 'E', 'R', 'T', 'Y' };
        REQUIRE_NOTHROW(ser->as_string("test_val", test_val5.data()));

        REQUIRE(obj.at("test_val").is_string());
        REQUIRE_EQ(obj.at("test_val").get<std::string>(), test_val5.data());
    }
}
