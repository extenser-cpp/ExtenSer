// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_HPP
#define EXTENSER_HPP

#include "detail/macros.hpp"
#include "detail/type_traits.hpp"
#include "span.hpp"

#include <array>
#include <cstddef>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace extenser
{
namespace detail
{
    template<typename F, typename... Ts>
    constexpr void for_each_tuple(const std::tuple<Ts...>& tuple, F&& func)
    {
        static_assert(std::conjunction_v<std::is_invocable<F, Ts>...>,
            "applied function must be able to take given args");

        std::apply([func = std::forward<F>(func)](auto&&... args)
            { (func(std::forward<decltype(args)>(args)), ...); },
            tuple);
    }
} //namespace detail

namespace containers
{
    template<typename Container>
    class adapter;

    template<typename Container>
    struct traits
    {
        using container_type = Container;
        using size_type = typename Container::size_type;
        using value_type = typename Container::value_type;
        using adapter_type = adapter<Container>;

        static constexpr bool has_fixed_size = false;
        static constexpr bool is_mutable = true;
        static constexpr bool is_sequential = true;
    };

    template<typename Container>
    struct sequential_traits : public traits<Container>
    {
        using typename traits<Container>::container_type;
        using typename traits<Container>::size_type;
        using typename traits<Container>::value_type;
        using typename traits<Container>::adapter_type;

        static constexpr bool is_contiguous = false;
    };

    template<typename Container>
    struct associative_traits : public traits<Container>
    {
        using typename traits<Container>::container_type;
        using typename traits<Container>::size_type;
        using key_type = typename Container::key_type;
        using mapped_type = typename Container::mapped_type;
        using typename traits<Container>::value_type;
        using typename traits<Container>::adapter_type;

        static constexpr bool is_sequential = false;
    };

    // TODO: Can this be a template-template of `traits<>`?
    template<typename Traits>
    class adapter_base
    {
    public:
        using adapter_type = typename Traits::adapter_type;
        using container_type = typename Traits::container_type;
        using size_type = typename Traits::size_type;

        EXTENSER_INLINE auto size(const container_type& container) const -> size_type
        {
            return (static_cast<const adapter_type*>(this))->size(container);
        }
    };

    template<typename Traits>
    class sequential_adapter : public adapter_base<Traits>
    {
    public:
        static_assert(
            Traits::is_sequential, "Only sequential containers can derive from sequential_adapter");

        using adapter_type = typename Traits::adapter_type;
        using container_type = typename Traits::container_type;
        using size_type = typename Traits::size_type;

        template<typename InputIt, typename ConversionOp>
        EXTENSER_INLINE void assign_from_range(
            container_type& container, InputIt first, InputIt last, ConversionOp convert_fn)
        {
            (static_cast<adapter_type*>(this))
                ->assign_from_range(container, first, last, convert_fn);
        }
    };

    template<typename Traits>
    class associative_adapter : public adapter_base<Traits>
    {
    public:
        static_assert(!Traits::is_sequential,
            "Sequential containers should not derive from associative_adapter");

        using adapter_type = typename Traits::adapter_type;
        using container_type = typename Traits::container_type;
        using size_type = typename Traits::size_type;
        using value_type = typename Traits::value_type;

        template<typename Input_T, typename ConversionOp>
        EXTENSER_INLINE void insert_value(
            container_type& container, const Input_T& value, ConversionOp convert_fn)
        {
            (static_cast<adapter_type*>(this))->insert_value(container, value, convert_fn);
        }
    };
} //namespace containers

// serialization constexpr checks
template<typename T>
inline constexpr bool is_bool_serializable = detail::is_boolean_testable_v<T>;

template<typename T>
inline constexpr bool is_float_serializable = std::is_floating_point_v<T>;

template<typename T>
inline constexpr bool is_int_serializable = std::is_integral_v<T> && std::is_signed_v<T>;

template<typename T>
inline constexpr bool is_uint_serializable = std::is_integral_v<T> && std::is_unsigned_v<T>;

template<typename T>
inline constexpr bool is_enum_serializable = std::is_enum_v<T>;

template<typename T>
inline constexpr bool is_string_serializable = detail::is_stringlike_v<T>;

template<typename T>
inline constexpr bool is_array_serializable = detail::is_container_v<T> || std::is_array_v<T>;

template<typename T>
inline constexpr bool is_map_serializable = detail::is_map_v<T>;

template<typename T>
inline constexpr bool is_multimap_serializable = detail::is_multimap_v<T>;

template<typename T>
inline constexpr bool is_tuple_serializable = detail::is_tuple_v<T> || detail::is_pair_v<T>;

template<typename T>
inline constexpr bool is_optional_serializable = detail::is_optional_v<T>;

template<typename T>
inline constexpr bool is_variant_serializable = detail::is_variant_v<T>;

template<typename T>
inline constexpr bool is_null_serializable = std::is_null_pointer_v<T> || std::is_void_v<T>
    || std::is_same_v<T, std::monostate> || std::is_same_v<T, std::nullopt_t>;

// TODO: check if there is a `serialize` function via ADL or static member
template<typename T>
inline constexpr bool is_object_serializable = true;

class extenser_exception : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class serialization_error : public extenser_exception
{
public:
    using extenser_exception::extenser_exception;
};

class deserialization_error : public extenser_exception
{
public:
    using extenser_exception::extenser_exception;
};

template<typename Derived>
class generic_serializer
{
public:
    generic_serializer() noexcept = default;
    generic_serializer& operator=(const generic_serializer&) = delete;
    generic_serializer& operator=(generic_serializer&&) = delete;

    template<typename T>
    EXTENSER_INLINE void as_bool(const std::string_view key, bool& val)
    {
        (static_cast<Derived*>(this))->as_bool(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_float(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_float(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_int(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_int(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_uint(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_uint(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_enum(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_enum(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_string(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_string(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_array(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_array(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_map(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_map(key, val);
    }

    template<typename T1, typename T2>
    EXTENSER_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
    {
        (static_cast<Derived*>(this))->as_tuple(key, val);
    }

    template<typename... Args>
    EXTENSER_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
    {
        (static_cast<Derived*>(this))->as_tuple(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
    {
        (static_cast<Derived*>(this))->as_optional(key, val);
    }

    template<typename... Args>
    EXTENSER_INLINE void as_variant(const std::string_view key, std::variant<Args...>& val)
    {
        (static_cast<Derived*>(this))->as_variant(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_object(const std::string_view key, T& val)
    {
        (static_cast<Derived*>(this))->as_object(key, val);
    }

    EXTENSER_INLINE void as_null(const std::string_view key)
    {
        (static_cast<Derived*>(this))->as_null(key);
    }

protected:
    ~generic_serializer() noexcept = default;
    generic_serializer(const generic_serializer&) = default;
    generic_serializer(generic_serializer&&) noexcept = default;
};

template<typename Adapter, bool Deserialize>
class serializer_base : public generic_serializer<serializer_base<Adapter, Deserialize>>
{
public:
    using serializer_t = std::conditional_t<Deserialize, typename Adapter::deserializer_t,
        typename Adapter::serializer_t>;

    static constexpr std::size_t max_variant_size = 10;

    template<typename T>
    void serialize_object(const T& val)
    {
        static_assert(!Deserialize, "Cannot call serialize_object() on a deserializer");
        static_assert(!std::is_pointer_v<T>,
            "Cannot serialize a pointer directly, wrap it in a span or view");

        // Necessary for bi-directional serialization
        serialize(*this, const_cast<T&>(val)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    template<typename T>
    void deserialize_object(T&& val)
    {
        static_assert(Deserialize, "Cannot call deserialize_object() on a serializer");
        static_assert(!std::is_pointer_v<T>,
            "Cannot serialize a pointer directly, wrap it in a span or view");
        serialize(*this, std::forward<T>(val));
    }

    EXTENSER_INLINE void as_bool(const std::string_view key, bool& val)
    {
        (static_cast<serializer_t*>(this))->as_bool(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_float(const std::string_view key, T& val)
    {
        static_assert(is_float_serializable<T>, "T must be a floating-point type");
        (static_cast<serializer_t*>(this))->as_float(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_int(const std::string_view key, T& val)
    {
        static_assert(is_int_serializable<T>, "T must be a signed integral type");
        (static_cast<serializer_t*>(this))->as_int(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_uint(const std::string_view key, T& val)
    {
        static_assert(is_uint_serializable<T>, "T must be an unsigned integral type");
        (static_cast<serializer_t*>(this))->as_uint(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_enum(const std::string_view key, T& val)
    {
        static_assert(is_enum_serializable<T>, "T must be an enum type");
        (static_cast<serializer_t*>(this))->as_enum(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_string(const std::string_view key, T& val)
    {
        static_assert(is_string_serializable<T>, "T must be convertible to std::string_view");
        (static_cast<serializer_t*>(this))->as_string(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_array(const std::string_view key, T& val)
    {
        static_assert(is_array_serializable<T>, "T must have begin() and end()");

        (static_cast<serializer_t*>(this))->as_array(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_map(const std::string_view key, T& val)
    {
        static_assert(
            is_map_serializable<T> || is_multimap_serializable<T>, "T must be a map type");

        if constexpr (is_multimap_serializable<T>)
        {
            (static_cast<serializer_t*>(this))->as_multimap(key, val);
        }
        else
        {
            (static_cast<serializer_t*>(this))->as_map(key, val);
        }
    }

    template<typename T1, typename T2>
    EXTENSER_INLINE void as_tuple(const std::string_view key, std::pair<T1, T2>& val)
    {
        (static_cast<serializer_t*>(this))->as_tuple(key, val);
    }

    template<typename... Args>
    EXTENSER_INLINE void as_tuple(const std::string_view key, std::tuple<Args...>& val)
    {
        (static_cast<serializer_t*>(this))->as_tuple(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_optional(const std::string_view key, std::optional<T>& val)
    {
        (static_cast<serializer_t*>(this))->as_optional(key, val);
    }

    template<typename... Args>
    EXTENSER_INLINE void as_variant(const std::string_view key, std::variant<Args...>& val)
    {
        static_assert(
            sizeof...(Args) < max_variant_size, "arg count can't exceed max_variant_size");

        (static_cast<serializer_t*>(this))->as_variant(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_object(const std::string_view key, T& val)
    {
        static_assert(is_object_serializable<T>, "serialize function for T could not be found");
        (static_cast<serializer_t*>(this))->as_object(key, val);
    }

    EXTENSER_INLINE void as_null(const std::string_view key)
    {
        (static_cast<serializer_t*>(this))->as_null(key);
    }
};

template<typename Adapter>
using serializer = typename Adapter::serializer_t;

template<typename Adapter>
using deserializer = typename Adapter::deserializer_t;

// Overloads for common types
template<typename Adapter>
void serialize(serializer_base<Adapter, false>& ser, const bool val)
{
    ser.as_bool("", val);
}

template<typename Adapter>
void serialize(serializer_base<Adapter, true>& ser, bool& val)
{
    ser.as_bool("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && std::is_signed_v<T> && (!std::is_same_v<T, bool>)),
        bool> = true>
void serialize(serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_int("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && std::is_signed_v<T> && (!std::is_same_v<T, bool>)),
        bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_int("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && std::is_unsigned_v<T> && (!std::is_same_v<T, bool>)),
        bool> = true>
void serialize(serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_uint("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && std::is_unsigned_v<T> && (!std::is_same_v<T, bool>)),
        bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_uint("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
void serialize(serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_enum("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_enum("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
void serialize(serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_float("", val);
}

template<typename Adapter, typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_float("", val);
}

template<typename Adapter>
void serialize(serializer_base<Adapter, false>& ser, const std::string_view val)
{
    ser.as_string("", val);
}

template<typename Adapter, typename T, std::enable_if_t<detail::is_stringlike_v<T>, bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_string("", val);
}

template<typename Adapter, bool Deserialize, typename T,
    std::enable_if_t<std::is_array_v<T>, bool> = true>
void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
{
    span arr_span{ val };
    ser.as_array("", arr_span);
}

template<typename Adapter, bool Deserialize, typename T,
    std::enable_if_t<detail::is_map_v<T>, bool> = true>
void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
{
    ser.as_map("", val);
}

// TODO: Change `containers` design so these can be moved to separate headers
template<typename Adapter, bool Deserialize, typename T1, typename T2>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::pair<T1, T2>& val)
{
    ser.as_tuple("", val);
}

template<typename Adapter, bool Deserialize, typename... Args>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::tuple<Args...>& val)
{
    ser.as_tuple("", val);
}

template<typename Adapter, bool Deserialize, typename T>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::optional<T>& val)
{
    ser.as_optional("", val);
}

template<typename Adapter, bool Deserialize>
void serialize(serializer_base<Adapter, Deserialize>& ser, [[maybe_unused]] std::nullptr_t& val)
{
    ser.as_null("");
}

template<typename Adapter, bool Deserialize>
void serialize(serializer_base<Adapter, Deserialize>& ser, [[maybe_unused]] std::nullopt_t& val)
{
    ser.as_null("");
}

template<typename Adapter, bool Deserialize>
void serialize(serializer_base<Adapter, Deserialize>& ser, [[maybe_unused]] std::monostate& val)
{
    ser.as_null("");
}

template<typename Adapter, bool Deserialize, typename... Args>
void serialize(serializer_base<Adapter, Deserialize>& ser, std::variant<Args...>& val)
{
    ser.as_variant("", val);
}
} //namespace extenser
#endif //EXTENSER_HPP
