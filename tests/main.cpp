#include "json/extenser_json.hpp"

#include <iostream>
#include <string>

struct Person
{
    int age{};
    std::string name{};
};

template<typename S>
void serialize(extenser::generic_serializer<S>& ser, Person& person)
{
    ser.as_int("age", person.age);
    ser.as_string("name", person.name);
}

int main()
{
    try
    {
        const Person in_person{ 42, "Jake" };
        extenser::serializer<extenser::json_adapter> ser{};
        ser.serialize_object(in_person);

        auto obj = std::move(ser).object();

        std::cout << obj.dump() << '\n';

        assert(obj.contains("age") && obj["age"] == 42);
        assert(obj.contains("name") && obj["name"] == "Jake");

        Person out_person{};
        extenser::deserializer<extenser::json_adapter> dser{ obj };
        dser.deserialize_object(out_person);

        assert(out_person.age == 42);
        assert(out_person.name == "Jake");
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
