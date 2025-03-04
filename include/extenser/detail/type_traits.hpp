// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_TYPE_TRAITS_HPP
#define EXTENSER_TYPE_TRAITS_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace extenser::detail
{
// TODO: Replace these type_traits w/ container traits

#define EXTENSER_NOPAREN(...) __VA_ARGS__
#define EXTENSER_CHECKER(NAME, EXPR1, EXPR2)                          \
    template<typename C>                                              \
    struct NAME                                                       \
    {                                                                 \
    private:                                                          \
        template<typename T>                                          \
        static constexpr auto check([[maybe_unused]] T* ptr) noexcept \
            -> std::is_same<decltype(EXPR1), EXPR2>;                  \
        template<typename>                                            \
        static constexpr auto check(...) noexcept -> std::false_type; \
        using type = decltype(check<C>(nullptr));                     \
                                                                      \
    public:                                                           \
        static constexpr bool value = type::value;                    \
    }

EXTENSER_CHECKER(has_begin, std::declval<T>().begin(), typename T::iterator);
EXTENSER_CHECKER(has_end, std::declval<T>().end(), typename T::iterator);
EXTENSER_CHECKER(has_size, std::declval<T>().size(), typename T::iterator);
EXTENSER_CHECKER(has_set_key, typename T::value_type{}, typename T::key_type);
EXTENSER_CHECKER(
    has_map_iterator, std::declval<typename T::iterator>()->second, typename T::mapped_type);

EXTENSER_CHECKER(has_map_at, std::declval<T>().at(typename T::key_type{}),
    std::add_lvalue_reference_t<typename T::mapped_type>);

#undef EXTENSER_CHECKER

#define EXTENSER_TYPE_TRAIT(NAME, COND)      \
    template<typename C>                     \
    struct NAME : std::bool_constant<(COND)> \
    {                                        \
    };                                       \
    template<typename C>                     \
    inline constexpr bool NAME##_v = NAME<C>::value

#define EXTENSER_CONJ_TYPE_TRAIT(NAME, ...) \
    EXTENSER_TYPE_TRAIT(NAME, std::conjunction<EXTENSER_NOPAREN(__VA_ARGS__)>::value)

#define EXTENSER_DISJ_TYPE_TRAIT(NAME, ...) \
    EXTENSER_TYPE_TRAIT(NAME, std::disjunction<EXTENSER_NOPAREN(__VA_ARGS__)>::value)

EXTENSER_TYPE_TRAIT(is_boolean_testable, (std::is_convertible_v<C, bool>));
EXTENSER_DISJ_TYPE_TRAIT(
    is_stringlike, std::is_convertible<C, std::string>, std::is_convertible<C, std::string_view>);

EXTENSER_CONJ_TYPE_TRAIT(
    is_container, has_begin<std::remove_cv_t<C>>, has_end<std::remove_cv_t<C>>);

EXTENSER_CONJ_TYPE_TRAIT(is_map, is_container<C>, has_map_iterator<std::remove_cv_t<C>>);
EXTENSER_CONJ_TYPE_TRAIT(is_multimap, is_map<C>, std::negation<has_map_at<C>>);
EXTENSER_CONJ_TYPE_TRAIT(is_set, is_container<C>, has_set_key<std::remove_cv_t<C>>);
#undef EXTENSER_TYPE_TRAIT
#undef EXTENSER_CONJ_TYPE_TRAIT
#undef EXTENSER_DISJ_TYPE_TRAIT

template<typename Adapter, bool Deserialize>
class serializer_base;

template<typename C>
struct has_serialize_adl
{
private:
    struct dummy_adapter
    {
        using bytes_t = void;
        using serial_t = void;
        using serializer_t = void;
        using deserializer_t = void;
        using config = void;
    };

    template<typename T>
    static constexpr auto check([[maybe_unused]] T* ptr) noexcept -> std::conjunction<
        std::is_same<decltype(serialize(std::declval<serializer_base<dummy_adapter, false>&>(),
                         std::declval<T&>())),
            void>,
        std::is_same<decltype(serialize(std::declval<serializer_base<dummy_adapter, true>&>(),
                         std::declval<T&>())),
            void>>;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename T>
inline constexpr bool has_serialize_adl_v = has_serialize_adl<T>::value;

template<typename C>
struct has_serialize_mem
{
private:
    struct dummy_adapter
    {
        using bytes_t = void;
        using serial_t = void;
        using serializer_t = void;
        using deserializer_t = void;
        using config = void;
    };

    template<typename T>
    static constexpr auto check([[maybe_unused]] T* ptr) noexcept
        -> std::conjunction<std::is_same<decltype(std::declval<T&>().serialize(std::declval<
                                             serializer_base<dummy_adapter, false>&>())),
                                void>,
            std::is_same<decltype(std::declval<T&>().serialize(
                             std::declval<serializer_base<dummy_adapter, true>&>())),
                void>>;

    template<typename>
    static constexpr std::false_type check(...) noexcept;

    using type = decltype(check<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

template<typename T>
inline constexpr bool has_serialize_mem_v = has_serialize_mem<T>::value;

template<typename T, typename It>
struct constructible_from_iterator :
    std::bool_constant<
        std::is_constructible_v<T, std::remove_reference_t<decltype(*std::declval<It&>())>>>
{
};

template<typename T, typename It>
inline constexpr bool constructible_from_iterator_v = constructible_from_iterator<T, It>::value;

#if defined(__cpp_lib_remove_cvref)
using std::remove_cvref;
using std::remove_cvref_t;
#else
// backport of C++20's remove_cvref
template<typename T>
struct remove_cvref
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;
#endif

#if defined(__cpp_lib_type_identity)
using std::type_identity;
using std::type_identity_t;
#else
// backport of C++20's type_identity
template<typename T>
struct type_identity
{
    using type = T;
};

template<typename T>
using type_identity_t = typename type_identity<T>::type;
#endif

template<typename T>
struct decay_str
{
    static_assert(!std::is_pointer_v<remove_cvref_t<T>>,
        "Pointer parameters are not allowed, please wrap in a span or view");
    static_assert(!std::is_array_v<remove_cvref_t<T>>,
        "C-style array parameters are not allowed, please wrap in a span or view");

    using type = T;
};

template<>
struct decay_str<const char*>
{
    using type = const std::string&;
};

template<>
struct decay_str<const wchar_t*>
{
    using type = const std::wstring&;
};

template<>
struct decay_str<const char16_t*>
{
    using type = const std::u16string&;
};

template<>
struct decay_str<const char32_t*>
{
    using type = const std::u32string&;
};

#if defined(__cpp_char8_t)
template<>
struct decay_str<const char8_t*>
{
    using type = const std::u8string&;
};
#endif

template<>
struct decay_str<const char*&>
{
    using type = const std::string&;
};

template<>
struct decay_str<const wchar_t*&>
{
    using type = const std::wstring&;
};

template<>
struct decay_str<const char16_t*&>
{
    using type = const std::u16string&;
};

template<>
struct decay_str<const char32_t*&>
{
    using type = const std::u32string&;
};

#if defined(__cpp_char8_t)
template<>
struct decay_str<const char8_t*&>
{
    using type = const std::u8string&;
};
#endif

template<>
struct decay_str<const char* const&>
{
    using type = const std::string&;
};

template<>
struct decay_str<const wchar_t* const&>
{
    using type = const std::wstring&;
};

template<>
struct decay_str<const char16_t* const&>
{
    using type = const std::u16string&;
};

template<>
struct decay_str<const char32_t* const&>
{
    using type = const std::u32string&;
};

#if defined(__cpp_char8_t)
template<>
struct decay_str<const char8_t* const&>
{
    using type = const std::u8string&;
};
#endif

template<std::size_t N>
struct decay_str<const char (&)[N]>
{
    using type = const std::string&;
};

template<std::size_t N>
struct decay_str<const wchar_t (&)[N]>
{
    using type = const std::wstring&;
};

template<std::size_t N>
struct decay_str<const char16_t (&)[N]>
{
    using type = const std::u16string&;
};

template<std::size_t N>
struct decay_str<const char32_t (&)[N]>
{
    using type = const std::u32string&;
};

#if defined(__cpp_char8_t)
template<std::size_t N>
struct decay_str<const char8_t (&)[N]>
{
    using type = const std::u8string&;
};
#endif

template<>
struct decay_str<std::string_view>
{
    using type = const std::string&;
};

template<>
struct decay_str<std::wstring_view>
{
    using type = const std::wstring&;
};

template<>
struct decay_str<std::u16string_view>
{
    using type = const std::u16string&;
};

template<>
struct decay_str<std::u32string_view>
{
    using type = const std::u32string&;
};

#if defined(__cpp_char8_t)
template<>
struct decay_str<std::u8string_view>
{
    using type = const std::u8string&;
};
#endif

template<typename T>
using decay_str_t = typename decay_str<T>::type;

template<typename C>
struct is_optional : std::false_type
{
};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type
{
};

template<typename C>
inline constexpr bool is_optional_v = is_optional<std::remove_cv_t<C>>::value;

template<typename C>
struct is_pair : std::false_type
{
};

template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type
{
};

template<typename C>
inline constexpr bool is_pair_v = is_pair<std::remove_cv_t<C>>::value;

template<typename C>
struct is_tuple : std::false_type
{
};

template<typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type
{
};

template<typename C>
inline constexpr bool is_tuple_v = is_tuple<std::remove_cv_t<C>>::value;

template<typename C>
struct is_variant : std::false_type
{
};

template<typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type
{
};

template<typename C>
inline constexpr bool is_variant_v = is_variant<std::remove_cv_t<C>>::value;
} // namespace extenser::detail
#endif
