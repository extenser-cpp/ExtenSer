# ExtenSer

An extensible, generic serialization library for C++.

## License

ExtenSer is licensed under the [BSD 3-Clause License](LICENSE).

### Third-Party

- [nlohmann-json](https://github.com/nlohmann/json) is licensed under the [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT).
- [bitsery](https://github.com/fraillt/bitsery) is licensed under the [MIT License](https://github.com/fraillt/bitsery/blob/master/LICENSE).

## Features

- Cross-platform.
  - Supports every major compiler/operating system.
- Easy to use.
  - Simply add a single function for each of your user-defined types.
- Modern.
  - Written with C++17, support for some C++20 types.
  - Takes advantage of a lot of compile-time and generic programming features.
- Support for various types.
  - C++ Standard Library:
    - Containers (`std::array`, `std::map`, `std::vector`, etc.).
      - Includes support for spans (either C++20's `std::span` or a C++17 backport: `extenser::span`).
    - Strings (`std::string`, `std::string_view`, `std::wstring`, etc.).
    - `std::optional`
    - `std::pair` / `std::tuple`
    - `std::variant`
  - Users can add a `serialize` member function for their own types.
    - Can also provide a non-member `template` function for serializing external types via ADL.
- Extensible support via "adapters".
  - Built-in JSON support using [nlohmann-json](https://github.com/nlohmann/json).
  - Optional project-supported adapters:
    - Binary serialization with [bitsery](https://github.com/fraillt/bitsery).
  - Community-supported adapters:
    - **More to come!**

## Examples

### Serializing Standard Types

```C++
#include "extenser/extenser.hpp"
#include "extenser/json_adapter/extenser_json.hpp"
#include "extenser/containers/string.hpp"

#include <cassert>
#include <optional>
#include <string>

int main()
{
    extenser::serializer<json_adapter> serializer{};
    extenser::deserializer<json_adapter> deserializer{ serializer.object() };
 
    std::string input_str = "Hello, world!";
    std::string output_str{};
 
    // Serialize one object (overwrites existing serialized data)
    serializer.serialize_object(input_str);

    // Deserialize one object
    deserializer.deserialize_object(output_str);

    assert(output_str == input_str);
    
    std::optional<int> input_opt = 22;
    std::optional<int> output_opt{};
    
    std::map<std::string, int> input_map = { {"John", 22}, {"Jane", 33} };
    std::map<std::string, int> output_map{};

    // manually clear state of the serializer
    serializer.reset();

    // Serialize multiple objects (does not overwrite)
    serializer.as_optional("opt", input_opt);
    serializer.as_map("map", input_map);
    
    // Deserialize multiple objects
    deserializer.as_optional("opt", output_opt);
    deserializer.as_map("map", output_map);
    
    assert(output_opt.has_value());
    assert(output_opt.value() == input_opt.value());
    
    assert(input_map == output_map);
}
```
### Serializing User Types

```C++
class Person
{
public:
    Person(int age, std::string name) : age(age), name(std::move(name))
    {
    }

    template<typename S>
    void serialize(extenser::generic_serializer<S>& ser)
    {
        ser.as_int("age", age);
        ser.as_string("name", name);
    }

private:
    int age;
    std::string name;
};
```
### Serializing Non-Accessible Types Via ADL

```C++
struct LibraryType
{
    int number;
    float ratio;
    std::vector<std::string> tags;
};
```

```C++
template<typename S>
void serialize(extenser::generic_serializer<S>& ser, LibraryType& value)
{
    ser.as_int("number", value.number);
    ser.as_float("ratio", value.ratio);
    ser.as_array("tags", value.tags);
}
```
