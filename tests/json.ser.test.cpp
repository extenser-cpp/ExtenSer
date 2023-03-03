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
TEST_SUITE("json::serializer")
{
    using serializer = json_adapter::serializer_t;

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

        const auto obj2 = std::move(ser).object();

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

        CHECK_NOTHROW(ser->as_bool("", true));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_boolean());
        REQUIRE(static_cast<bool>(obj));

        ser.emplace();

        CHECK_NOTHROW(ser->as_bool("test_val", false));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE_FALSE(static_cast<bool>(sub_obj));

        CHECK_NOTHROW(ser->as_bool("test_val", 22));
        REQUIRE(static_cast<bool>(sub_obj));

        CHECK_NOTHROW(ser->as_bool("test_val", nullptr));
        REQUIRE_FALSE(static_cast<bool>(sub_obj));
    }

    TEST_CASE("as_float")
    {
        static constexpr double test_epsilon{ 0.0001 };
        static constexpr float test_val1{ std::numeric_limits<float>::min() };
        static constexpr double test_val2{ std::numeric_limits<double>::quiet_NaN() };
        static constexpr double test_val3{ 3.141592653589793 };
        static constexpr int64_t test_val4_i{ 1'234'567LL };
        static constexpr double test_val4{ test_val4_i };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_float("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_float());
        REQUIRE_EQ(obj.get<float>(), doctest::Approx(test_val1).epsilon(test_epsilon));

        ser.emplace();

        CHECK_NOTHROW(ser->as_float("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE(sub_obj.is_number_float());
        REQUIRE(doctest::IsNaN(sub_obj.get<double>()));

        CHECK_NOTHROW(ser->as_float("test_val", test_val3));
        REQUIRE(sub_obj.is_number_float());
        REQUIRE_EQ(sub_obj.get<long double>(), doctest::Approx(test_val3).epsilon(test_epsilon));

        CHECK_NOTHROW(ser->as_float("test_val", test_val4_i));
        REQUIRE(sub_obj.is_number_float());
        REQUIRE_EQ(sub_obj.get<double>(), doctest::Approx(test_val4).epsilon(test_epsilon));
    }

    TEST_CASE("as_int")
    {
        static constexpr int test_val1{ 12'345 };
        static constexpr intmax_t test_val2{ std::numeric_limits<intmax_t>::min() };
        static constexpr int8_t test_val3{ 0x4A };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_int("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE_FALSE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<int>(), test_val1);

        ser.emplace();

        CHECK_NOTHROW(ser->as_int("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE(sub_obj.is_number_integer());
        REQUIRE_FALSE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<intmax_t>(), test_val2);

        CHECK_NOTHROW(ser->as_int("test_val", test_val3));
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE_FALSE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<int8_t>(), test_val3);
    }

    TEST_CASE("as_uint")
    {
        static constexpr unsigned test_val1{ 12'345 };
        static constexpr uintmax_t test_val2{ std::numeric_limits<uintmax_t>::max() };
        static constexpr uint8_t test_val3{ 0x4A };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_uint("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<unsigned>(), test_val1);

        ser.emplace();

        CHECK_NOTHROW(ser->as_uint("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE(sub_obj.is_number_integer());
        REQUIRE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<uintmax_t>(), test_val2);

        CHECK_NOTHROW(ser->as_uint("test_val", test_val3));
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<uint8_t>(), test_val3);
    }

    TEST_CASE("as_enum")
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

        static constexpr Fruit test_val1{ Fruit::Pineapple };
        static constexpr TestCode test_val2{ TestCode::CodeB };
        static constexpr PlainEnum test_val3{ VALUE_XX };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_enum("", test_val1));
        REQUIRE_FALSE(obj.empty());

#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE(obj.is_string());
        REQUIRE_EQ(obj.get<std::string>(), magic_enum::enum_name<Fruit>(test_val1));
#else
        REQUIRE(obj.is_number_integer());
        REQUIRE_FALSE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<Fruit>(), test_val1);
#endif

        ser.emplace();

        CHECK_NOTHROW(ser->as_enum("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), magic_enum::enum_name<TestCode>(test_val2));
#else
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<TestCode>(), test_val2);
#endif

        CHECK_NOTHROW(ser->as_enum("test_val", test_val3));
#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), magic_enum::enum_name<PlainEnum>(test_val3));
#else
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE_FALSE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<PlainEnum>(), test_val3);
#endif

#if defined(EXTENSER_USE_MAGIC_ENUM)
        static constexpr TestCode test_val4{ 0xCCU };

        REQUIRE_THROWS(ser->as_enum("test_val", test_val4));
#endif
    }

    TEST_CASE("as_string")
    {
        static constexpr std::string_view test_val1{ "Hello, world!" };
        static constexpr const char* test_val2{ "Mary had a little lamb" };
        static constexpr char test_val3[]{ "Whose fleece was white as snow" }; //NOLINT
        static constexpr std::array<char, 6> test_val4{ 'Q', 'W', 'E', 'R', 'T', 'Y' };
        const std::string test_val5{ "Goodbye, cruel world!" };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_string("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_string());
        REQUIRE_EQ(obj.get<std::string>(), test_val1);

        ser.emplace();

        CHECK_NOTHROW(ser->as_string("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), test_val2);

        CHECK_NOTHROW(ser->as_string("test_val", test_val3));
        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), test_val3);

        CHECK_NOTHROW(ser->as_string("test_val", test_val4.data()));
        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), test_val4.data());

        CHECK_NOTHROW(ser->as_string("test_val", test_val5));
        REQUIRE(sub_obj.is_string());
        REQUIRE_EQ(sub_obj.get<std::string>(), test_val5);
    }

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

            std::optional<serializer> ser{};
            ser.emplace();
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

            std::optional<serializer> ser{};
            ser.emplace();
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

            std::optional<serializer> ser{};
            ser.emplace();
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

    TEST_CASE("as_map")
    {
        const std::map<int, std::string> test_val1{ { 33, "Benjamin Burton" },
            { 99, "John Johnson" }, { 444, "Reed Carmichael" } };
        const std::unordered_map<std::string, Person> test_val2{
            { "Henrietta",
                Person{ 16, "Henrietta Payne", {}, Pet{ "Ron", Pet::Species::Fish }, {} } },
            { "Jerome", Person{ 12, "Jerome Banks", {}, {}, {} } },
            { "Rachel", Person{ 22, "Rachel Franks", {}, {}, {} } },
            { "Ricardo",
                Person{ 19, "Ricardo Montoya", {}, Pet{ "Sinbad", Pet::Species::Cat }, {} } }
        };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_map("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE_EQ(obj.size(), test_val1.size());

        for (const auto& [k, v] : test_val1)
        {
            const auto key_str = std::to_string(k);

            REQUIRE(obj.contains(key_str));
            REQUIRE(obj[key_str].is_string());
            REQUIRE_EQ(obj[key_str], v);
        }

        ser.emplace();

        CHECK_NOTHROW(ser->as_map("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE(sub_obj.is_object());
        REQUIRE_EQ(sub_obj.size(), test_val2.size());

        for (const auto& [k, v] : test_val2)
        {
            REQUIRE(sub_obj.contains(k));
            REQUIRE(sub_obj[k].is_object());
            REQUIRE(sub_obj[k].contains("age"));
            REQUIRE(sub_obj[k]["age"].is_number_integer());
            REQUIRE_EQ(sub_obj[k]["age"], v.age);
        }
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
        std::optional<serializer> ser{};
        ser.emplace();
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

    TEST_CASE("as_object")
    {
        const Person test_val1_friend{ 10, "Timmy Johnson", {},
            { Pet{ "Sparky", Pet::Species::Dog } }, { { Fruit::Banana, 2 }, { Fruit::Apple, 2 } } };

        const Person test_val1{ 22, "Franky Johnson", { test_val1_friend },
            { Pet{ "Tommy", Pet::Species::Turtle } },
            { { Fruit::Apple, 1 }, { Fruit::Mango, 2 } } };

        const Person test_val2{ 44, "Bertha Jenkins", {}, {}, { { Fruit::Kiwi, 12 } } };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_object("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());

        REQUIRE(obj.contains("age"));
        REQUIRE(obj["age"].is_number_integer());
        REQUIRE_FALSE(obj["age"].is_number_unsigned());
        REQUIRE_EQ(obj["age"].get<int>(), test_val1.age);

        REQUIRE(obj.contains("name"));
        REQUIRE(obj["name"].is_string());
        REQUIRE_EQ(obj["name"].get<std::string>(), test_val1.name);

        REQUIRE(obj.contains("friends"));
        REQUIRE(obj["friends"].is_array());
        REQUIRE_EQ(obj["friends"].size(), test_val1.friends.size());
        REQUIRE_EQ(obj["friends"][0]["age"], test_val1.friends[0].age);

        REQUIRE(obj.contains("pet"));
        REQUIRE(obj["pet"].is_object());
        REQUIRE(obj["pet"].contains("name"));
        REQUIRE(obj["pet"]["name"].is_string());
        REQUIRE_EQ(obj["pet"]["name"].get<std::string>(), test_val1.pet->name);
        REQUIRE(obj["pet"].contains("species"));
#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE(obj["pet"]["species"].is_string());
        REQUIRE_EQ(obj["pet"]["species"].get<std::string>(),
            magic_enum::enum_name<Pet::Species>(test_val1.pet->species));
#else
        REQUIRE(obj["pet"]["species"].is_number_integer());
        REQUIRE_EQ(obj["pet"]["species"].get<int>(), static_cast<int>(test_val1.pet->species));
#endif

        REQUIRE(obj.contains("fruit_count"));
        REQUIRE(obj["fruit_count"].is_object());
#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE(obj["fruit_count"].contains("Apple"));
        REQUIRE(obj["fruit_count"]["Apple"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["Apple"], test_val1.fruit_count.at(Fruit::Apple));
        REQUIRE(obj["fruit_count"].contains("Mango"));
        REQUIRE(obj["fruit_count"]["Mango"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["Mango"], test_val1.fruit_count.at(Fruit::Mango));
#else
        REQUIRE(obj["fruit_count"].contains("0"));
        REQUIRE(obj["fruit_count"]["0"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["0"], test_val1.fruit_count.at(Fruit::Apple));
        REQUIRE(obj["fruit_count"].contains("4"));
        REQUIRE(obj["fruit_count"]["4"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["4"], test_val1.fruit_count.at(Fruit::Mango));
#endif

        ser.emplace();

        CHECK_NOTHROW(ser->as_object("test_val", test_val2));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_object());
        REQUIRE(obj.contains("test_val"));

        const auto& sub_obj = obj.at("test_val");

        REQUIRE_FALSE(sub_obj.empty());
        REQUIRE(sub_obj.is_object());

        REQUIRE(sub_obj.contains("age"));
        REQUIRE(sub_obj["age"].is_number_integer());
        REQUIRE_FALSE(sub_obj["age"].is_number_unsigned());
        REQUIRE_EQ(sub_obj["age"].get<int>(), test_val2.age);

        REQUIRE(sub_obj.contains("name"));
        REQUIRE(sub_obj["name"].is_string());
        REQUIRE_EQ(sub_obj["name"].get<std::string>(), test_val2.name);

        REQUIRE(sub_obj.contains("friends"));
        REQUIRE(sub_obj["friends"].is_array());
        REQUIRE(sub_obj["friends"].empty());

        REQUIRE(sub_obj.contains("pet"));
        REQUIRE(sub_obj["pet"].is_null());

        REQUIRE(sub_obj.contains("fruit_count"));
        REQUIRE(sub_obj["fruit_count"].is_object());
#if defined(EXTENSER_USE_MAGIC_ENUM)
        REQUIRE_FALSE(sub_obj["fruit_count"].contains("Apple"));
        REQUIRE(sub_obj["fruit_count"].contains("Kiwi"));
        REQUIRE(sub_obj["fruit_count"]["Kiwi"].is_number_integer());
        REQUIRE_EQ(sub_obj["fruit_count"]["Kiwi"], test_val2.fruit_count.at(Fruit::Kiwi));
#else
        REQUIRE_FALSE(sub_obj["fruit_count"].contains("0"));
        REQUIRE(sub_obj["fruit_count"].contains("3"));
        REQUIRE(sub_obj["fruit_count"]["3"].is_number_integer());
        REQUIRE_EQ(sub_obj["fruit_count"]["3"], test_val2.fruit_count.at(Fruit::Kiwi));
#endif
    }
}
} //namespace extenser::tests
