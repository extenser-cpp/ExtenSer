#ifndef EXTENSER_JSON_TEST_HPP
#define EXTENSER_JSON_TEST_HPP

#include "extenser.hpp"
#include "json/extenser_json.hpp"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <optional>
#include <string>
#include <unordered_map>
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
};
} //namespace extenser::tests
#endif //EXTENSER_JSON_TEST_HPP
