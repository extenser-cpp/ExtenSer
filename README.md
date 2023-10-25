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

struct NoDefault
{
public:
    NoDefault() = delete;
    NoDefault(int num) : number(num) {}

    template<typename S>
    void serialize(extenser::generic_serializer<S>& ser)
    {
        ser.as_int("", number);
    }

    int number;
};

int main()
{
    extenser::easy_serializer<json_adapter> serializer{};

    // Serialize default constructible type
    const std::string input_str = "Hello, world!";
    serializer.serialize_object(input_str);

    const auto output_str = serializer.deserialize_object<std::string>();
    assert(output_str == input_str);

    // Serialize non-default constructible type
    NotDefault input_nd(2);
    serializer.serialize_object(input_nd);
    
    NotDefault out_nd(1);
    serializer.deserialize_object(out_nd);
    assert(out_nd.number == 2);
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
