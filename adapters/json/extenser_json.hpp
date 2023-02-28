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

#ifndef EXTENSER_JSON_HPP
#define EXTENSER_JSON_HPP

#include "extenser.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
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
    };

    class serializer : public serializer_base<serial_adapter, false>
    {
    public:
        serializer() noexcept = default;

        [[nodiscard]] auto object() const& -> const nlohmann::json&
        {
            EXTENSER_POSTCONDITION(m_json.is_null() || !m_json.empty());
            return m_json;
        }

        [[nodiscard]] auto object() && -> nlohmann::json&&
        {
            EXTENSER_POSTCONDITION(m_json.is_null() || !m_json.empty());
            return std::move(m_json);
        }

        template<typename T>
        void as_bool(const std::string_view key, const T& val)
        {
            subobject(key) = static_cast<bool>(val);
        }

        template<typename T>
        void as_float(const std::string_view key, const T& val)
        {
            static_assert(!std::is_same_v<T, long double>, "long double is not supported");
            subobject(key) = static_cast<double>(val);
        }

        template<typename T>
        void as_int(const std::string_view key, const T& val)
        {
            static_assert(std::is_signed_v<T> || !std::is_integral_v<T>,
                "only signed integers are supported");
            static_assert(sizeof(T) <= sizeof(int64_t) || !std::is_integral_v<T>,
                "maximum 64-bit integers supported");

            if constexpr (std::is_integral_v<T>)
            {
                subobject(key) = val;
            }
            else
            {
                subobject(key) = static_cast<int64_t>(val);
            }
        }

        template<typename T>
        void as_uint(const std::string_view key, const T& val)
        {
            static_assert(std::is_unsigned_v<T> || !std::is_integral_v<T>,
                "only unsigned integers are supported");
            static_assert(sizeof(T) <= sizeof(int64_t) || !std::is_integral_v<T>,
                "maximum 64-bit integers supported");

            if constexpr (std::is_integral_v<T>)
            {
                subobject(key) = val;
            }
            else
            {
                subobject(key) = static_cast<uint64_t>(val);
            }
        }

        template<typename T>
        void as_string(const std::string_view key, const T& val)
        {
            static_assert(detail::is_stringlike_v<T>, "T must be convertible to std::string_view");
            subobject(key) = val;
        }

        template<typename T>
        void as_array(const std::string_view key, const T& val)
        {
            auto arr = nlohmann::json::array();

            for (const auto& subval : val)
            {
                push_args(subval, arr);
            }

            subobject(key) = std::move(arr);
        }

        template<typename T>
        void as_map(const std::string_view key, const T& val)
        {
            auto obj = nlohmann::json::object();

            for (const auto& [k, v] : val)
            {
                const auto key_str = nlohmann::json{ k }.front().dump();
                obj[key_str] = v;
            }

            subobject(key) = std::move(obj);
        }

        template<typename T>
        void as_multimap(const std::string_view key, const T& val)
        {
            auto obj = nlohmann::json::object();

            for (const auto& [k, v] : val)
            {
                const auto key_str = nlohmann::json{ k }.dump();

                if (obj.find(key_str) == end(obj))
                {
                    obj[key_str] = nlohmann::json::array();
                }

                obj[key_str].push_back(v);
            }

            subobject(key) = std::move(obj);
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, const std::pair<T1, T2>& val)
        {
            auto obj = nlohmann::json::object();
            obj["first"] = val.first;
            obj["second"] = val.second;
            subobject(key) = std::move(obj);
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, const std::tuple<Args...>& val)
        {
            // Need to create the subobject in case args is empty
            auto& arg_arr = subobject(key);

            detail::for_each_tuple(val,
                [&arg_arr](auto&& elem)
                { push_args(std::forward<decltype(elem)>(elem), arg_arr); });
        }

        template<typename T>
        void as_optional(const std::string_view key, const std::optional<T>& val)
        {
            if (val.has_value())
            {
                auto& sub_obj = subobject(key);
                push_arg(*val, sub_obj);
            }
            else
            {
                subobject(key) = nullptr;
            }
        }

        template<typename... Args>
        void as_variant(const std::string_view key, const std::variant<Args...>& val)
        {
            auto& new_arg = subobject(key);

            new_arg["v_idx"] = val.index();
            auto& var_val = new_arg["v_val"];
            std::visit([&var_val](auto&& l_val)
                { push_arg(std::forward<decltype(l_val)>(l_val), var_val); },
                val);
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
        static void push_arg(T&& arg, nlohmann::json& obj)
        {
            serializer ser{};
            ser.serialize_object(std::forward<T>(arg));
            obj = std::move(ser).object();
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

    class deserializer : public serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const nlohmann::json& obj) : m_json(obj)
        {
            EXTENSER_POSTCONDITION(m_json.is_null() || !m_json.empty());
        }

        template<typename T>
        void as_bool(const std::string_view key, T& val) const
        {
            val = subobject(key).get<bool>();
        }

        template<typename T>
        void as_float(const std::string_view key, T& val) const
        {
            val = subobject(key).get<T>();
        }

        template<typename T>
        void as_int(const std::string_view key, T& val) const
        {
            val = subobject(key).get<T>();
        }

        template<typename T>
        void as_uint(const std::string_view key, T& val) const
        {
            val = subobject(key).get<T>();
        }

        template<typename T>
        void as_string(const std::string_view key, T& val) const
        {
            val = subobject(key).get<std::string>();
        }

        template<typename T>
        void as_array(const std::string_view key, T& val) const
        {
            const auto& arr = subobject(key);
            val = T{ cbegin(arr), cend(arr) };
        }

        template<typename T>
        void as_array(const std::string_view key, span<T>& val) const
        {
            const auto& arr = subobject(key);

            if (arr.size() != val.size())
            {
                throw std::out_of_range{ "JSON error: array out of bounds" };
            }

            if constexpr (!std::is_const_v<T>)
            {
                std::copy(cbegin(arr), cend(arr), std::begin(val));
            }
        }

        template<typename T>
        void as_map(const std::string_view key, T& val) const
        {
            EXTENSER_PRECONDITION(std::size(val) == 0);

            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                val.insert(
                    { nlohmann::json::parse(k).front().template get<typename T::key_type>(), v });
            }
        }

        template<typename T>
        void as_multimap(const std::string_view key, T& val) const
        {
            EXTENSER_PRECONDITION(std::size(val) == 0);

            const auto& obj = subobject(key);

            for (const auto& [k, v] : obj.items())
            {
                for (const auto& subval : v)
                {
                    val.insert(
                        { nlohmann::json::parse(k).front().template get<typename T::key_type>(),
                            subval });
                }
            }
        }

        template<typename T1, typename T2>
        void as_tuple(const std::string_view key, std::pair<T1, T2>& val) const
        {
            const auto& obj = subobject(key);
            val = { obj.at("first"), obj.at("second") };
        }

        template<typename... Args>
        void as_tuple(const std::string_view key, std::tuple<Args...>& val) const
        {
            if (subobject(key).size() != sizeof...(Args))
            {
                throw std::runtime_error{ "JSON error: invalid number of args" };
            }

            [[maybe_unused]] size_t arg_counter = 0;
            val = { parse_args<Args>(subobject(key), arg_counter)... };
        }

        template<typename T>
        void as_optional(const std::string_view key, std::optional<T>& val) const
        {
            const auto& obj = subobject(key);
            val = obj.is_null() ? std::optional<T>{ std::nullopt }
                                : std::optional<T>{ std::in_place, obj.template get<T>() };
        }

        template<typename... Args>
        void as_variant(const std::string_view key, std::variant<Args...>& val) const
        {
            static constexpr size_t arg_sz = sizeof...(Args);

            const auto& obj = subobject(key);
            const auto v_idx = obj.at("v_idx").get<size_t>();

            if (v_idx >= arg_sz)
            {
                throw std::runtime_error{
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

                default:
                    EXTENSER_ASSUME(0);
            }
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
            EXTENSER_PRECONDITION(key.empty() || m_json.is_object());
            return key.empty() ? m_json : m_json.at(key);
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
                return arg.is_number() && (!arg.is_number_float());
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return arg.is_number_float();
            }
            else if constexpr (detail::is_stringlike_v<T>)
            {
                return arg.is_string();
            }
            else if constexpr (detail::is_map_v<T>)
            {
                return arg.is_object();
            }
            else if constexpr (detail::is_container_v<T>)
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
                return !arg.is_null();
            }
        }

        template<typename T>
        [[nodiscard]] static auto parse_arg(const nlohmann::json& arg)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            using no_ref_t = detail::remove_cvref_t<detail::decay_str_t<T>>;

            if (!validate_arg<no_ref_t>(arg))
            {
#ifdef EXTENSER_NO_RTTI
                throw std::runtime_error{
                    std::string{ "JSON error: expected type: {NO-RTTI}, got type: " }.append(
                        arg.type_name())
                };
#else
                throw std::runtime_error{ std::string{ "JSON error: expected type: " }
                                              .append(typeid(no_ref_t).name())
                                              .append(", got type: ")
                                              .append(arg.type_name()) };
#endif
            }

            no_ref_t out_val;
            deserializer ser{ arg };
            ser.deserialize_object(out_val);
            return out_val;
        }

        template<typename T>
        [[nodiscard]] static auto parse_args(const nlohmann::json& arg_arr, size_t& index)
            -> detail::remove_cvref_t<detail::decay_str_t<T>>
        {
            if (index >= arg_arr.size())
            {
                throw std::runtime_error{ "JSON error: argument count mismatch" };
            }

            if (arg_arr.is_array())
            {
                const auto old_idx = index;
                ++index;
                return parse_arg<T>(arg_arr[old_idx]);
            }

            return parse_arg<T>(arg_arr);
        }

        const nlohmann::json& m_json;
    };
} //namespace detail_json

using json_adapter = detail_json::serial_adapter;
} //namespace extenser
#endif //EXTENSER_JSON_HPP
