// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_JSON_HPP
#define EXTENSER_JSON_HPP

#include <extenser/extenser.hpp>

#if defined(EXTENSER_USE_MAGIC_ENUM)
#  if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    include <magic_enum.hpp>
#    pragma GCC diagnostic pop
#  else
#    include <magic_enum.hpp>
#  endif
#endif

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#ifndef EXTENSER_NO_RTTI
#  include <typeinfo>
#endif

namespace extenser
{
template<typename Adapter, bool Deserialize>
void serialize(serializer_base<Adapter, Deserialize>& ser, nlohmann::json& obj)
{
    ser.as_object("", obj);
}

namespace detail_json
{
    class serializer;
    class deserializer;

    struct serial_adapter
    {
        using bytes_t = std::string;
        using serial_t = nlohmann::json;
        using serializer_t = serializer;
        using deserializer_t = deserializer;
        using config = void;

        [[nodiscard]] static auto to_bytes(const nlohmann::json& obj) -> std::string
        {
            return obj.dump();
        }

        [[nodiscard]] static auto to_bytes(nlohmann::json&& obj) -> std::string
        {
            return std::move(obj).dump();
        }

        [[nodiscard]] static auto from_bytes(const std::string& str) -> nlohmann::json
        {
            return nlohmann::json::parse(str);
        }

        [[nodiscard]] static auto from_bytes(std::string&& str) -> nlohmann::json
        {
            return nlohmann::json::parse(std::move(str));
        }
    };

    class serializer : public detail::serializer_base<serial_adapter, false>
    {
    public:
        serializer() noexcept = default;

        [[nodiscard]] auto object() const& noexcept(::EXTENSER_ASSERT_NOTHROW)
            -> const nlohmann::json&
        {
            EXTENSER_POSTCONDITION(m_json.is_null() || !m_json.empty());
            return m_json;
        }

        [[nodiscard]] auto object() && noexcept(::EXTENSER_ASSERT_NOTHROW) -> nlohmann::json&&
        {
            EXTENSER_POSTCONDITION(m_json.is_null() || !m_json.empty());
            return std::move(m_json);
        }

        void as_bool(const std::string_view key, const bool val) noexcept
        {
            push_simple_type(val, subobject(key));
        }

        template<typename T>
        void as_float(const std::string_view key, const T val)
        {
            static_assert(!std::is_same_v<T, long double>, "long double is not supported");
            push_simple_type(static_cast<double>(val), subobject(key));
        }

        template<typename T>
        void as_int(const std::string_view key, const T val)
        {
            static_assert(sizeof(T) <= sizeof(std::int64_t), "maximum 64-bit integers supported");
            static_assert(
                std::is_integral_v<T> && std::is_signed_v<T>, "only signed integers are supported");

            push_simple_type(val, subobject(key));
        }

        template<typename T>
        void as_uint(const std::string_view key, const T val)
        {
            static_assert(sizeof(T) <= sizeof(std::int64_t), "maximum 64-bit integers supported");
            static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                "only unsigned integers are supported");

            push_simple_type(val, subobject(key));
        }

        template<typename T>
        void as_enum(const std::string_view key, const T val)
        {
            static_assert(std::is_enum_v<T>, "T must be an enum type");
            push_enum(val, subobject(key));
        }

        template<typename T>
        void as_string(const std::string_view key, const T& val)
        {
            //static_assert(detail::is_stringlike_v<T>, "T must be convertible to std::string_view");
            push_string(val, subobject(key));
        }

        template<typename T>
        void as_array(const std::string_view key, const T& val)
        {
            push_array(val, subobject(key));
        }

        template<typename T>
        void as_map(const std::string_view key, const T& val)
        {
            push_map(val, subobject(key));
        }

        template<typename T>
        void as_multimap(const std::string_view key, const T& val)
        {
            push_multimap(val, subobject(key));
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, const std::pair<T1, T2>& val)
        {
            push_pair(val, subobject(key));
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, const std::tuple<Args...>& val)
        {
            push_tuple(val, subobject(key));
        }

        template<typename T>
        void as_optional(const std::string_view key, const std::optional<T>& val)
        {
            push_optional(val, subobject(key));
        }

        template<typename... Args>
        void as_variant(const std::string_view key, const std::variant<Args...>& val)
        {
            push_variant(val, subobject(key));
        }

        void as_object(const std::string_view key, const nlohmann::json& val)
        {
            subobject(key) = val;
        }

        template<typename T>
        void as_object(const std::string_view key, const T& val)
        {
            push_arg(val, subobject(key));
        }

        void as_null(const std::string_view key) { subobject(key) = nullptr; }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) -> nlohmann::json&
        {
            return key.empty() ? m_json : m_json[key];
        }

        template<typename T>
        static void push_simple_type(const T arg, nlohmann::json& obj) noexcept
        {
            obj = arg;
        }

        template<typename T>
        static void push_enum(const T arg, nlohmann::json& obj)
        {
            using no_ref_t = detail::remove_cvref_t<T>;

#if defined(EXTENSER_USE_MAGIC_ENUM)
            if (!magic_enum::enum_contains<no_ref_t>(arg))
            {
                throw serialization_error{
                    std::string{ "Invalid enum value: " }
                        .append(std::to_string(static_cast<std::underlying_type_t<no_ref_t>>(arg)))
                        .append(" for type: ")
                        .append(magic_enum::enum_type_name<no_ref_t>())
                };
            }

            push_string(magic_enum::enum_name<no_ref_t>(arg), obj);
#else
            push_simple_type(static_cast<std::underlying_type_t<no_ref_t>>(arg), obj);
#endif
        }

        static void push_string(const std::string_view arg, nlohmann::json& obj) { obj = arg; }
        static void push_string(const std::wstring_view arg, nlohmann::json& obj) { obj = arg; }
        static void push_string(const std::u16string_view arg, nlohmann::json& obj) { obj = arg; }
        static void push_string(const std::u32string_view arg, nlohmann::json& obj) { obj = arg; }
#if defined(__cpp_char8_t)
        static void push_string(const std::u8string_view arg, nlohmann::json& obj) { obj = arg; }
#endif

        template<typename T>
        static void push_array(T&& arg, nlohmann::json& obj)
        {
            obj = nlohmann::json::array();

            for (const auto& subval : std::forward<T>(arg))
            {
                push_args(subval, obj);
            }
        }

        template<typename T>
        static void push_map(T&& arg, nlohmann::json& obj)
        {
            obj = nlohmann::json::object();

            for (const auto& [k, v] : std::forward<T>(arg))
            {
                const std::string key_str = stringize_key(k);

                auto& val_obj = obj[key_str];
                push_arg(v, val_obj);
            }
        }

        template<typename T>
        static void push_multimap(T&& arg, nlohmann::json& obj)
        {
            obj = nlohmann::json::object();

            for (const auto& [k, v] : std::forward<T>(arg))
            {
                const std::string key_str = stringize_key(k);

                if (obj.find(key_str) == end(obj))
                {
                    obj[key_str] = nlohmann::json::array();
                }

                push_args(v, obj[key_str]);
            }
        }

        template<typename T1, typename T2>
        static void push_pair(const std::pair<T1, T2>& arg, nlohmann::json& obj)
        {
            obj = nlohmann::json::array();
            push_args(arg.first, obj);
            push_args(arg.second, obj);
        }

        template<typename... Args>
        static void push_tuple(const std::tuple<Args...>& arg, nlohmann::json& obj)
        {
            obj = nlohmann::json::array();
            detail::for_each_tuple(
                arg, [&obj](auto&& elem) { push_args(std::forward<decltype(elem)>(elem), obj); });
        }

        template<typename T>
        static void push_optional(const std::optional<T>& arg, nlohmann::json& obj)
        {
            if (arg.has_value())
            {
                push_arg(*arg, obj);
            }
            else
            {
                obj = nullptr;
            }
        }

        template<typename... Args>
        static void push_variant(const std::variant<Args...>& arg, nlohmann::json& obj)
        {
            obj["v_idx"] = arg.index();
            auto& var_val = obj["v_val"];

            std::visit([&var_val](auto&& l_val)
                { push_arg(std::forward<decltype(l_val)>(l_val), var_val); },
                arg);
        }

        template<typename T>
        static void push_object(T&& arg, nlohmann::json& obj)
        {
            serializer ser{};
            ser.serialize_object(std::forward<T>(arg));
            obj = std::move(ser).object();
        }

        template<typename T>
        static auto stringize_key(T&& key_arg) -> std::string
        {
            nlohmann::json key_obj{};
            push_arg(std::forward<T>(key_arg), key_obj);

            if (!key_obj.is_string())
            {
                return "@" + key_obj.dump();
            }

            auto key_str = key_obj.get<std::string>();

            if (key_str.front() == '@')
            {
                // Escape '@' at front of string
                key_str.insert(key_str.begin(), '@');
            }

            return key_str;
        }

        template<typename T>
        static void push_arg(T&& arg, nlohmann::json& obj)
        {
            using no_ref_t = detail::remove_cvref_t<T>;

            if constexpr (is_null_serializable<no_ref_t>)
            {
                obj = nullptr;
            }
            else if constexpr (std::is_arithmetic_v<no_ref_t>)
            {
                push_simple_type(std::forward<T>(arg), obj);
            }
            else if constexpr (is_enum_serializable<no_ref_t>)
            {
                push_enum(std::forward<T>(arg), obj);
            }
            else if constexpr (is_string_serializable<no_ref_t>)
            {
                push_string(std::forward<T>(arg), obj);
            }
            else if constexpr (is_array_serializable<no_ref_t>)
            {
                push_array(std::forward<T>(arg), obj);
            }
            else if constexpr (is_multimap_serializable<no_ref_t>)
            {
                push_multimap(std::forward<T>(arg), obj);
            }
            else if constexpr (is_map_serializable<no_ref_t>)
            {
                push_map(std::forward<T>(arg), obj);
            }
            else if constexpr (is_optional_serializable<no_ref_t>)
            {
                push_optional(arg, obj);
            }
            else if constexpr (is_tuple_serializable<no_ref_t>)
            {
                if constexpr (detail::is_pair_v<no_ref_t>)
                {
                    push_pair(arg, obj);
                }
                else
                {
                    push_tuple(arg, obj);
                }
            }
            else if constexpr (is_variant_serializable<no_ref_t>)
            {
                push_variant(arg, obj);
            }
            else
            {
                push_object(std::forward<T>(arg), obj);
            }
        }

        template<typename T>
        static void push_args(T&& arg, nlohmann::json& obj_arr)
        {
            nlohmann::json tmp{};
            push_arg(std::forward<T>(arg), tmp);
            obj_arr.push_back(std::move(tmp));
        }

        nlohmann::json m_json{};
    };

    class deserializer : public detail::serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const nlohmann::json& obj) noexcept : m_p_json(&obj)
        {
        }

        void as_bool(const std::string_view key, bool& val) const
        {
            try
            {
                val = subobject(key).get<bool>();
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        void as_float(const std::string_view key, T& val) const
        {
            try
            {
                val = subobject(key).get<T>();
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        void as_int(const std::string_view key, T& val) const
        {
            try
            {
                val = subobject(key).get<T>();
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        void as_uint(const std::string_view key, T& val) const
        {
            try
            {
                val = subobject(key).get<T>();
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        void as_enum(const std::string_view key, T& val) const
        {
            static_assert(std::is_enum_v<T>, "T must be an enum type");

            try
            {
#if defined(EXTENSER_USE_MAGIC_ENUM)
                auto result = magic_enum::enum_cast<T>(subobject(key).get<std::string>());

                if (!result.has_value())
                {
                    throw deserialization_error{ std::string{ "Invalid enum value: \"" }
                                                     .append(subobject(key).get<std::string>())
                                                     .append("\" for type: ")
                                                     .append(magic_enum::enum_type_name<T>()) };
                }

                val = *result;
#else
                val = static_cast<T>(subobject(key).get<std::underlying_type_t<T>>());
#endif
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        void as_string(const std::string_view key, T& val) const
        {
            using traits_t = containers::string_traits<T>;
            using adapter_t = containers::adapter<T>;

            if constexpr (std::is_same_v<T, std::string>)
            {
                try
                {
                    val = subobject(key).get<std::string>();
                }
                catch (const deserialization_error&)
                {
                    throw;
                }
                catch (const std::exception& ex)
                {
                    throw deserialization_error{ ex.what() };
                }
            }
            else
            {
                if constexpr (traits_t::is_mutable)
                {
                    const auto& sub_obj = subobject(key);

                    if constexpr (std::is_same_v<typename traits_t::character_type, char>)
                    {
                        const auto str = sub_obj.get<std::string>();

                        if constexpr (traits_t::has_fixed_size)
                        {
                            if (str.size() != adapter_t::size(val))
                            {
                                throw deserialization_error{ "JSON error: array out of bounds" };
                            }
                        }

                        adapter_t::assign_from_range(val, str.begin(), str.end(),
                            [](const char c)
                            { return static_cast<typename traits_t::value_type>(c); });
                    }
                    else
                    {
                        if constexpr (traits_t::has_fixed_size)
                        {
                            if (sub_obj.size() != adapter_t::size(val))
                            {
                                throw deserialization_error{ "JSON error: array out of bounds" };
                            }
                        }

                        adapter_t::assign_from_range(val, sub_obj.begin(), sub_obj.end(),
                            [](const nlohmann::json& sub_val)
                            { return sub_val.get<typename traits_t::value_type>(); });
                    }
                }
                else
                {
                    std::ignore = key;
                    std::ignore = val;
                }
            }
        }

        template<typename T>
        void as_array(const std::string_view key, T& val) const
        {
            using traits_t = containers::traits<T>;
            using adapter_t = containers::adapter<T>;

            if constexpr (traits_t::is_mutable)
            {
                const auto& arr = subobject(key);

                if constexpr (traits_t::has_fixed_size)
                {
                    if (arr.size() != adapter_t::size(val))
                    {
                        throw deserialization_error{ "JSON error: array out of bounds" };
                    }
                }

                if constexpr (traits_t::is_sequential)
                {
                    adapter_t::assign_from_range(
                        val, arr.cbegin(), arr.cend(), parse_arg<typename traits_t::value_type>);
                }
                else
                {
                    for (const auto& j_obj : arr)
                    {
                        adapter_t::insert_value(
                            val, j_obj, parse_arg<typename traits_t::value_type>);
                    }
                }
            }
            else
            {
                std::ignore = key;
                std::ignore = val;
            }
        }

        template<typename T>
        void as_map(const std::string_view key, T& val) const
        {
            EXTENSER_PRECONDITION(std::size(val) == 0);

            using traits_t = containers::associative_traits<T>;
            using adapter_t = containers::adapter<T>;

            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                adapter_t::insert_value(val, std::make_pair(k, v),
                    parse_kv_pair<typename traits_t::key_type, typename traits_t::mapped_type>);
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, T& val) const
        {
            EXTENSER_PRECONDITION(std::size(val) == 0);

            using traits_t = containers::associative_traits<T>;
            using adapter_t = containers::adapter<T>;

            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                for (const auto& subval : v)
                {
                    adapter_t::insert_value(val, std::make_pair(k, subval),
                        parse_kv_pair<typename traits_t::key_type, typename traits_t::mapped_type>);
                }
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, std::pair<T1, T2>& val) const
        {
            const auto& obj = subobject(key);
            val = { parse_arg<T1>(obj.at(0)), parse_arg<T2>(obj.at(1)) };
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, std::tuple<Args...>& val) const
        {
            if (subobject(key).size() != sizeof...(Args))
            {
                throw deserialization_error{ "JSON error: invalid number of args" };
            }

            [[maybe_unused]] std::size_t arg_counter = 0;
            val = { parse_args<Args>(subobject(key), arg_counter)... };
        }

        template<typename T>
        void as_optional(const std::string_view key, std::optional<T>& val) const
        {
            const auto& obj = subobject(key);
            val = obj.is_null() ? std::optional<T>{ std::nullopt }
                                : std::optional<T>{ std::in_place, parse_arg<T>(obj) };
        }

        template<typename... Args>
        void as_variant(const std::string_view key, std::variant<Args...>& val) const
        {
            static constexpr std::size_t arg_sz = sizeof...(Args);
            static_assert(arg_sz <= max_variant_size, "Variant limit reached");

            const auto& obj = subobject(key);
            const auto v_idx = obj.at("v_idx").get<std::size_t>();

            if (v_idx >= arg_sz)
            {
                throw deserialization_error{
                    std::string{ "JSON error: variant index exceeded variant size: " }.append(
                        std::to_string(arg_sz))
                };
            }

            // TODO: Cleanup and move to serial-agnostic code
            switch (v_idx)
            {
                case 0:
                    val = parse_arg<decltype(std::get<0>(val))>(obj.at("v_val"));
                    return;

                case 1:
                    if constexpr (arg_sz > 1)
                    {
                        val = parse_arg<decltype(std::get<1>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 2:
                    if constexpr (arg_sz > 2)
                    {
                        val = parse_arg<decltype(std::get<2>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 3:
                    if constexpr (arg_sz > 3)
                    {
                        val = parse_arg<decltype(std::get<3>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 4:
                    if constexpr (arg_sz > 4)
                    {
                        val = parse_arg<decltype(std::get<4>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 5:
                    if constexpr (arg_sz > 5)
                    {
                        val = parse_arg<decltype(std::get<5>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 6:
                    if constexpr (arg_sz > 6)
                    {
                        val = parse_arg<decltype(std::get<6>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 7:
                    if constexpr (arg_sz > 7)
                    {
                        val = parse_arg<decltype(std::get<7>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 8:
                    if constexpr (arg_sz > 8)
                    {
                        val = parse_arg<decltype(std::get<8>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 9:
                    if constexpr (arg_sz > 9)
                    {
                        val = parse_arg<decltype(std::get<9>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 10:
                    if constexpr (arg_sz > 10)
                    {
                        val = parse_arg<decltype(std::get<10>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 11:
                    if constexpr (arg_sz > 11)
                    {
                        val = parse_arg<decltype(std::get<11>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 12:
                    if constexpr (arg_sz > 12)
                    {
                        val = parse_arg<decltype(std::get<12>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                case 13:
                    if constexpr (arg_sz > 13)
                    {
                        val = parse_arg<decltype(std::get<13>(val))>(obj.at("v_val"));
                        return;
                    }
                    [[fallthrough]];

                default:
                    EXTENSER_ASSUME(0);
            }
        }

        void as_object(const std::string_view key, nlohmann::json& val) const
        {
            val = subobject(key);
        }

        template<typename T>
        void as_object(const std::string_view key, T& val) const
        {
            val = parse_arg<T>(subobject(key));
        }

        void as_null([[maybe_unused]] const std::string_view key) const
        {
            EXTENSER_PRECONDITION(subobject(key).is_null());
        }

    private:
        [[nodiscard]] auto subobject(const std::string_view key) const -> const nlohmann::json&
        {
            EXTENSER_PRECONDITION(key.empty() || m_p_json->is_object());

            try
            {
                return key.empty() ? *m_p_json : m_p_json->at(key);
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        template<typename T>
        [[nodiscard]] static constexpr auto validate_arg(const nlohmann::json& arg) noexcept -> bool
        {
            if constexpr (detail::is_optional_v<T>)
            {
                return arg.is_null() || validate_arg<typename T::value_type>(arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return arg.is_boolean();
            }
            else if constexpr (std::is_integral_v<T>)
            {
                return arg.is_number_integer();
            }
            else if constexpr (std::is_enum_v<T>)
            {
#if defined(EXTENSER_USE_MAGIC_ENUM)
                return arg.is_string();
#else
                return arg.is_number_integer();
#endif
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return arg.is_number_float();
            }
            else if constexpr (detail::is_stringlike_v<T>)
            {
                return arg.is_string();
            }
            else if constexpr (detail::is_map_v<T> || detail::is_pair_v<T>)
            {
                return arg.is_object();
            }
            else if constexpr (detail::is_container_v<T> || detail::is_tuple_v<T>)
            {
                return arg.is_array();
            }
            else if constexpr (std::is_same_v<T, std::nullptr_t>
                || std::is_same_v<T, std::monostate> || std::is_same_v<T, std::nullopt_t>)
            {
                return arg.is_null();
            }
            else
            {
                return arg.is_object();
            }
        }

        [[nodiscard]] static auto parse_key_str(std::string_view key_str) -> nlohmann::json
        {
            if (key_str.front() == '@')
            {
                if (key_str.size() <= 1 || key_str[1] != '@')
                {
                    return nlohmann::json::parse(std::next(key_str.begin()), key_str.end()).front();
                }

                // Escaped '@' in string value
                key_str.remove_prefix(1);
            }

            return key_str;
        }

        template<typename Key, typename Value>
        [[nodiscard]] static auto parse_kv_pair(
            const std::pair<std::string, nlohmann::json>& kv_pair) -> std::pair<Key, Value>
        {
            const auto& [k, v] = kv_pair;
            const nlohmann::json key_obj = parse_key_str(k);

            return { parse_arg<Key>(key_obj), parse_arg<Value>(v) };
        }

        template<typename T>
        [[nodiscard]] static auto parse_arg(const nlohmann::json& arg)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

            if (!validate_arg<no_ref_t>(arg))
            {
#if defined(EXTENSER_NO_RTTI)
                throw deserialization_error{
                    std::string{ "JSON error: expected type: {NO-RTTI}, got type: " }.append(
                        arg.type_name())
                };
#else
                throw deserialization_error{ std::string{ "JSON error: expected type: " }
                                                 .append(typeid(no_ref_t).name())
                                                 .append(", got type: ")
                                                 .append(arg.type_name()) };
#endif
            }

            no_ref_t out_val;
            deserializer ser{ arg };

            try
            {
                ser.deserialize_object(out_val);
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }

            return out_val;
        }

        [[nodiscard]] static auto get_next_arg(
            const nlohmann::json& arg, std::size_t& index) noexcept -> const nlohmann::json&
        {
            if (arg.is_array())
            {
                const auto old_idx = index;
                ++index;

                return arg[old_idx];
            }

            return arg;
        }

        template<typename T>
        [[nodiscard]] static auto parse_args(const nlohmann::json& arg_arr, std::size_t& index)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            if (index >= arg_arr.size())
            {
                throw deserialization_error{ "JSON error: argument count mismatch" };
            }

            const auto& next_obj = get_next_arg(arg_arr, index);

            try
            {
                return parse_arg<T>(next_obj);
            }
            catch (const deserialization_error&)
            {
                throw;
            }
            catch (const std::exception& ex)
            {
                throw deserialization_error{ ex.what() };
            }
        }

        const nlohmann::json* m_p_json;
    };
} //namespace detail_json

using json_adapter = detail_json::serial_adapter;
} //namespace extenser
#endif //EXTENSER_JSON_HPP
