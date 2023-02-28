#include "extenser.hpp"
#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
enum class Fruit
{
    Apple,
    Banana,
    Grape,
    Kiwi,
    Mango,
    Orange,
    Pineapple,
    Strawberry,
};

struct Pet
{
    enum class Species
    {
        Bird,
        Cat,
        Dog,
        Fish,
        Snake,
        Turtle,
    };

    std::string name;
    Species species;
};

template<typename S>
void serialize(extenser::generic_serializer<S>& ser, Pet& pet)
{
    ser.as_string("name", pet.name);
    ser.as_int("species", pet.species);
}

struct Person
{
    int age;
    std::string name;
    std::vector<Person> friends;
    std::optional<Pet> pet;
    std::unordered_map<Fruit, int> fruit_count;
};

template<typename S>
void serialize(extenser::generic_serializer<S>& ser, Person& person)
{
    ser.as_int("age", person.age);
    ser.as_string("name", person.name);
    ser.as_array("friends", person.friends);
    ser.as_optional("pet", person.pet);
    ser.as_map("fruit_count", person.fruit_count);
}

auto create_3d_vec(size_t x_sz, size_t y_sz, size_t z_sz)
{
    std::vector<std::vector<std::vector<double>>> x;
    x.reserve(x_sz);

    for (size_t i = 0; i < x_sz; ++i)
    {
        std::vector<std::vector<double>> y;
        y.reserve(y_sz);

        for (size_t j = 0; j < y_sz; ++j)
        {
            std::vector<double> z;
            z.reserve(z_sz);

            for (size_t k = 0; k < z_sz; ++k)
            {
                z.push_back(static_cast<double>(k * j * i) * 0.333);
            }

            y.push_back(std::move(z));
        }

        x.push_back(std::move(y));
    }

    return x;
};
} //namespace

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

        CHECK_NOTHROW(ser->as_bool("", true));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_boolean());
        REQUIRE(static_cast<bool>(obj));

        ser.reset();
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
        static constexpr double test_val2{ NAN };
        static constexpr double test_val3{ M_PI };
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

        ser.reset();
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
        static constexpr double test_val4_f{ 12.34 };
        static constexpr int16_t test_val4{ static_cast<int16_t>(test_val4_f) };

        std::optional<serializer> ser{};
        ser.emplace();
        const auto& obj = ser.value().object();

        CHECK_NOTHROW(ser->as_int("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE_FALSE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<int>(), test_val1);

        ser.reset();
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

        CHECK_NOTHROW(ser->as_int("test_val", test_val4_f));
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE_FALSE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<int16_t>(), test_val4);
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

        CHECK_NOTHROW(ser->as_uint("", test_val1));
        REQUIRE_FALSE(obj.empty());
        REQUIRE(obj.is_number());
        REQUIRE(obj.is_number_integer());
        REQUIRE(obj.is_number_unsigned());
        REQUIRE_EQ(obj.get<unsigned>(), test_val1);

        ser.reset();
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

        CHECK_NOTHROW(ser->as_uint("test_val", test_val4_f));
        REQUIRE(sub_obj.is_number_integer());
        REQUIRE(sub_obj.is_number_unsigned());
        REQUIRE_EQ(sub_obj.get<uint16_t>(), test_val4);
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

        ser.reset();
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

            ser.reset();
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
            REQUIRE(std::equal(sub_obj.begin(), sub_obj.end(), test_val5.begin()));
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

            ser.reset();
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

        SUBCASE("array of maps") {}
        SUBCASE("array of tuples") {}
        SUBCASE("array of optionals") {}
        SUBCASE("array of variants") {}
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

            for (int i = 0; i < test_val1.size(); ++i)
            {
                REQUIRE_FALSE(obj[i].empty());
                REQUIRE(obj[i].is_object());
                REQUIRE(obj[i].contains("age"));
                REQUIRE(obj[i]["age"].is_number_integer());
                REQUIRE_EQ(obj[i]["age"].get<int>(), test_val1[i].age);
            }
        }
    }

    TEST_CASE("as_map") {}
    TEST_CASE("as_multimap") {}
    TEST_CASE("as_tuple") {}
    TEST_CASE("as_optional") {}
    TEST_CASE("as_variant") {}

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
        REQUIRE(obj["pet"]["species"].is_number_integer());
        REQUIRE_EQ(obj["pet"]["species"].get<int>(), static_cast<int>(test_val1.pet->species));

        REQUIRE(obj.contains("fruit_count"));
        REQUIRE(obj["fruit_count"].is_object());
        REQUIRE(obj["fruit_count"].contains("0"));
        REQUIRE(obj["fruit_count"]["0"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["0"], test_val1.fruit_count.at(Fruit::Apple));
        REQUIRE(obj["fruit_count"].contains("4"));
        REQUIRE(obj["fruit_count"]["4"].is_number_integer());
        REQUIRE_EQ(obj["fruit_count"]["4"], test_val1.fruit_count.at(Fruit::Mango));

        ser.reset();
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
        REQUIRE_FALSE(sub_obj["fruit_count"].contains("0"));
        REQUIRE(sub_obj["fruit_count"].contains("3"));
        REQUIRE(sub_obj["fruit_count"]["3"].is_number_integer());
        REQUIRE_EQ(sub_obj["fruit_count"]["3"], test_val2.fruit_count.at(Fruit::Kiwi));
    }
}

TEST_SUITE("json::deserializer")
{
    TEST_CASE("CTOR") {}
    TEST_CASE("object") {}
    TEST_CASE("as_bool") {}
    TEST_CASE("as_float") {}
    TEST_CASE("as_int") {}
    TEST_CASE("as_uint") {}
    TEST_CASE("as_string") {}

    TEST_CASE("as_array")
    {
        SUBCASE("array of values") {}
        SUBCASE("array of arrays") {}
        SUBCASE("array of maps") {}
        SUBCASE("array of tuples") {}
        SUBCASE("array of optionals") {}
        SUBCASE("array of variants") {}
        SUBCASE("array of objects") {}
    }

    TEST_CASE("as_map") {}
    TEST_CASE("as_multimap") {}
    TEST_CASE("as_tuple") {}
    TEST_CASE("as_optional") {}
    TEST_CASE("as_variant") {}
    TEST_CASE("as_object") {}
}
