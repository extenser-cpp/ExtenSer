#ifndef EXTENSER_JSON_TEST_HPP
#define EXTENSER_JSON_TEST_HPP

#include "extenser.hpp"
#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <deque>
#include <forward_list>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace extenser::tests
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

    std::string name{};
    Species species{};
};

template<typename S>
void serialize(extenser::generic_serializer<S>& ser, Pet& pet)
{
    ser.as_string("name", pet.name);
    ser.as_enum("species", pet.species);
}

struct Person
{
    int age{};
    std::string name{};
    std::vector<Person> friends{};
    std::optional<Pet> pet{};
    std::unordered_map<Fruit, int> fruit_count{};
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

inline auto create_3d_vec(size_t x_sz, size_t y_sz, size_t z_sz)
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
}

template<typename T>
auto create_test_val() -> T
{
    throw std::logic_error{ "Implementation missing" };
}

template<>
inline auto create_test_val<std::array<int, 5>>() -> std::array<int, 5>
{
    return { 1, 2, 3, 4, 5 };
}

template<>
inline auto create_test_val<std::string_view>() -> std::string_view
{
    return "Mary had a little lamb";
}

template<>
inline auto create_test_val<std::vector<bool>>() -> std::vector<bool>
{
    return { true, false, false, true, true, false, true, false, false, true, true, true, true,
        false, false, false };
}

template<>
inline auto create_test_val<std::deque<std::vector<double>>>() -> std::deque<std::vector<double>>
{
    return { { 1, 1, 1, 1, 1 }, { 1, 2, 3, 4, 5 }, { 4, 6, 8, 9, 19 }, { -1, -3, 12, 13, 10 },
        { 0, 0, 0, 0, 0 } };
}

template<>
inline auto create_test_val<std::list<Person>>() -> std::list<Person>
{
    const Person person1{ 10, "Timmy Johnson", {}, { Pet{ "Sparky", Pet::Species::Dog } },
        { { Fruit::Banana, 2 }, { Fruit::Apple, 2 } } };
    const Person person2{ 22, "Franky Johnson", { person1 },
        { Pet{ "Tommy", Pet::Species::Turtle } }, { { Fruit::Apple, 1 }, { Fruit::Mango, 2 } } };
    const Person person3{ 44, "Bertha Jenkins", {}, {}, { { Fruit::Kiwi, 12 } } };

    return { person1, person2, person3 };
}

template<>
inline auto create_test_val<std::forward_list<std::string>>() -> std::forward_list<std::string>
{
    return { "Mary had a little lamb", "whose fleece", "was white as snow" };
}

template<>
inline auto create_test_val<std::set<int>>() -> std::set<int>
{
    return { 1, 22, 333, 4444, 55555, 666666 };
}

template<>
inline auto create_test_val<std::unordered_multiset<std::string>>()
    -> std::unordered_multiset<std::string>
{
    return { "Red", "Green", "Red", "Black", "Blue", "Green", "Purple" };
}
} //namespace extenser::tests
#endif //EXTENSER_JSON_TEST_HPP