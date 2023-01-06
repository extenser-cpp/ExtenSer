//BSD 3-Clause License
//
//Copyright (c) 2023, Jackson Harmer
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
//3. Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef EXTENSER_HPP
#define EXTENSER_HPP

#include <cassert>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#if defined(_MSC_VER)
#  define EXTENSER_ASSUME(EXPR) __assume(EXPR)
#elif defined(__clang__)
#  define EXTENSER_ASSUME(EXPR) __builtin_assume(EXPR)
#elif defined(__GNUC__)
#  define EXTENSER_ASSUME(EXPR) (EXPR) ? static_cast<void>(0) : __builtin_unreachable()
#else
#  define EXTENSER_ASSUME(EXPR) static_cast<void>(0)
#endif

#if defined(EXTENSER_ASSERT_NONE)
#  define EXTENSER_ASSERTION(EXPR) static_cast<void>(0)
#elif defined(EXTENSER_ASSERT_DEBUG)
#  define EXTENSER_ASSERTION(EXPR) assert(EXPR)
#elif defined(EXTENSER_ASSERT_STDERR)
#  define EXTENSER_ASSERTION(EXPR)                                                             \
    if (!(EXPR))                                                                              \
    std::fprintf(stderr,                                                                      \
        "EXTENSER_ASSERTION: \"%s\" failed!\n  func: %s,\n  file: %s,\n  line: %d\n\n", #EXPR, \
        __FUNCTION__, __FILE__, __LINE__)
#elif defined(EXTENSER_ASSERT_THROW)
#  define EXTENSER_ASSERTION(EXPR) \
    if (!(EXPR))                  \
    throw std::runtime_error("EXTENSER_ASSERTION: \"" #EXPR "\" failed!")
#elif defined(EXTENSER_ASSERT_ABORT)
#  define EXTENSER_ASSERTION(EXPR) \
    if (!(EXPR))                  \
    std::abort()
#elif defined(EXTENSER_ASSERT_ASSUME)
#  define EXTENSER_ASSERTION(EXPR) EXTENSER_ASSUME(EXPR)
#else
#  define EXTENSER_ASSERTION(EXPR) assert(EXPR)
#endif

#define EXTENSER_POSTCONDITION(EXPR) EXTENSER_ASSERTION(EXPR)
#define EXTENSER_PRECONDITION(EXPR) EXTENSER_ASSERTION(EXPR)

#if defined(__GNUC__)
#  define EXTENSER_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER)
#  define EXTENSER_INLINE __forceinline
#else
#  define EXTENSER_INLINE
#endif

namespace extenser
{
template<typename Derived>
class generic_serializer
{
public:
    generic_serializer() noexcept = default;
    generic_serializer& operator=(const generic_serializer&) = delete;
    generic_serializer& operator=(generic_serializer&&) = delete;

    template<typename T>
    EXTENSER_INLINE void as_bool(const std::string_view key, T& val)
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

    static constexpr size_t max_variant_size = 10;

    template<typename T>
    void serialize_object(const T& val)
    {
        static_assert(!Deserialize, "Cannot call serialize_object() on a deserializer");

        // Necessary for bi-directional serialization
        serialize(*this, const_cast<T&>(val)); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    template<typename T>
    void deserialize_object(T&& val)
    {
        static_assert(Deserialize, "Cannot call deserialize_object() on a serializer");
        serialize(*this, std::forward<T>(val));
    }

    template<typename T>
    EXTENSER_INLINE void as_bool(const std::string_view key, T& val)
    {
        static_assert(detail::is_boolean_testable_v<T>, "T must be convertible to bool");
        (static_cast<serializer_t*>(this))->as_bool(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_float(const std::string_view key, T& val)
    {
        static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");
        (static_cast<serializer_t*>(this))->as_float(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_int(const std::string_view key, T& val)
    {
        static_assert(std::is_integral_v<T> || std::is_enum_v<T>, "T must be an integral type");
        (static_cast<serializer_t*>(this))->as_int(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_string(const std::string_view key, T& val)
    {
        static_assert(detail::is_stringlike_v<T>, "T must be convertible to std::string_view");
        (static_cast<serializer_t*>(this))->as_string(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_array(const std::string_view key, T& val)
    {
        static_assert(detail::is_container_v<T>, "T must have begin() and end()");
        (static_cast<serializer_t*>(this))->as_array(key, val);
    }

    template<typename T>
    EXTENSER_INLINE void as_map(const std::string_view key, T& val)
    {
        static_assert(detail::is_map_v<T>, "T must be a map type");

        if constexpr (detail::is_multimap_v<T>)
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
        (static_cast<serializer_t*>(this))->as_object(key, val);
    }

    EXTENSER_INLINE void as_null(const std::string_view key)
    {
        (static_cast<serializer_t*>(this))->as_null(key);
    }
};

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
    std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
void serialize(serializer_base<Adapter, false>& ser, const T val)
{
    ser.as_int("", val);
}

template<typename Adapter, typename T,
    std::enable_if_t<(std::is_integral_v<T> && (!std::is_same_v<T, bool>)), bool> = true>
void serialize(serializer_base<Adapter, true>& ser, T& val)
{
    ser.as_int("", val);
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
    std::enable_if_t<(detail::is_container_v<T>
                         && (!detail::is_stringlike_v<T>)&&(!detail::is_map_v<T>)),
        bool> = true>
void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
{
    ser.as_array("", val);
}

template<typename Adapter, bool Deserialize, typename T,
    std::enable_if_t<detail::is_map_v<T>, bool> = true>
void serialize(serializer_base<Adapter, Deserialize>& ser, T& val)
{
    ser.as_map("", val);
}

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
