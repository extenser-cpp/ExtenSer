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

#ifndef EXTENSER_BITSERY_HPP
#define EXTENSER_BITSERY_HPP

#include "extenser.hpp"

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_optional.h>
#include <bitsery/ext/std_set.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/ext/std_variant.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/core/traits.h>
#include <bitsery/traits/forward_list.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template<>
struct std::hash<std::vector<uint8_t>>
{
    [[nodiscard]] auto operator()(const std::vector<uint8_t>& vec) const noexcept -> size_t
    {
        return std::accumulate(vec.begin(), vec.end(), vec.size(),
            [](const size_t seed, const size_t val) noexcept
            {
                static constexpr size_t magic_hash_val = 0x9E37'79B9UL;
                return seed ^ (val + magic_hash_val + (seed << 6UL) + (seed >> 2UL));
            });
    }
};

namespace extenser
{
// TODO: Define ContainerTraits for extenser::span

namespace detail_bitsery
{
    class serializer;
    class deserializer;

    struct serial_adapter
    {
        using bytes_t = std::vector<uint8_t>;
        using serial_t = std::vector<uint8_t>;
        using serializer_t = serializer;
        using deserializer_t = deserializer;

        struct config
        {
#if defined(EXTENSER_BITSERY_EXACT_SZ)
            static constexpr bool use_exact_size = true;
#else
            static constexpr bool use_exact_size = false;
#endif

            static const size_t max_string_size;
            static const size_t max_container_size;
        };

        template<typename S, typename T, typename Adapter, bool Deserialize>
        static void parse_obj(S& ser, serializer_base<Adapter, Deserialize>& fallback, T& val);
    };

    class serializer : public serializer_base<serial_adapter, false>
    {
    public:
        serializer() noexcept : m_ser(m_bytes) { m_bytes.reserve(64UL); }

        [[nodiscard]] auto object() & -> const std::vector<uint8_t>&
        {
            flush();

            EXTENSER_POSTCONDITION(!m_bytes.empty());
            return m_bytes;
        }

        [[nodiscard]] auto object() && -> std::vector<uint8_t>&&
        {
            flush();

            EXTENSER_POSTCONDITION(!m_bytes.empty());
            return std::move(m_bytes);
        }

        template<typename T>
        void as_bool([[maybe_unused]] const std::string_view key, const T& val)
        {
            m_ser.value1b(val);
        }

        template<typename T>
        void as_float([[maybe_unused]] const std::string_view key, const T& val)
        {
            static_assert(sizeof(T) <= sizeof(double), "long double not supported");
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_int([[maybe_unused]] const std::string_view key, const T& val)
        {
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_string([[maybe_unused]] const std::string_view key, const T& val)
        {
            m_ser.text1b(val, config::max_string_size);
        }

        template<typename T>
        void as_array([[maybe_unused]] const std::string_view key, const T& val)
        {
            using val_t = typename T::value_type;

            if constexpr (std::is_arithmetic_v<val_t>)
            {
                m_ser.container<sizeof(val_t)>(val, config::max_container_size);
            }
            else
            {
                m_ser.container(val, config::max_container_size);
            }
        }

        template<typename T, size_t N>
        void as_array([[maybe_unused]] const std::string_view key, const std::array<T, N>& val)
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                m_ser.container<sizeof(T)>(val);
            }
            else
            {
                m_ser.container(val);
            }
        }

        template<typename T>
        void as_map([[maybe_unused]] const std::string_view key, const T& val)
        {
            using S = bitsery::Serializer<output_adapter>;
            using key_t = typename T::key_type;
            using val_t = typename T::mapped_type;

            m_ser.ext(val, bitsery::ext::StdMap{ config::max_container_size },
                [this](S& ser, key_t& map_key, val_t& map_val)
                {
                    serial_adapter::parse_obj(ser, *this, map_key);
                    serial_adapter::parse_obj(ser, *this, map_val);
                });
        }

        template<typename T>
        void as_multimap([[maybe_unused]] const std::string_view key, const T& val)
        {
            as_map(key, val);
        }

        template<typename T1, typename T2>
        void as_tuple([[maybe_unused]] const std::string_view key, const std::pair<T1, T2>& val)
        {
            serial_adapter::parse_obj(m_ser, *this, val.first);
            serial_adapter::parse_obj(m_ser, *this, val.second);
        }

        template<typename... Args>
        void as_tuple([[maybe_unused]] const std::string_view key, const std::tuple<Args...>& val)
        {
            using S = bitsery::Serializer<output_adapter>;

            m_ser.ext(val,
                bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                    {
                        serial_adapter::parse_obj(ser, *this, subval);
                    } });
        }

        template<typename T>
        void as_optional([[maybe_unused]] const std::string_view key, const std::optional<T>& val)
        {
            using S = bitsery::Serializer<output_adapter>;

            m_ser.ext(val, bitsery::ext::StdOptional{},
                [this](S& ser, T& subval) { serial_adapter::parse_obj(ser, *this, subval); });
        }

        template<typename... Args>
        void as_variant(
            [[maybe_unused]] const std::string_view key, const std::variant<Args...>& val)
        {
            using S = bitsery::Serializer<output_adapter>;

            m_ser.ext(val,
                bitsery::ext::StdVariant{ [this](S& ser, auto& subval)
                    {
                        serial_adapter::parse_obj(ser, *this, subval);
                    } });
        }

        template<typename T>
        void as_object([[maybe_unused]] const std::string_view key, const T& val)
        {
            serial_adapter::parse_obj(m_ser, *this, val);
        }

        void as_null([[maybe_unused]] const std::string_view key)
        {
            // nop
        }

    private:
        using config = typename serial_adapter::config;
        using output_adapter = bitsery::OutputBufferAdapter<std::vector<uint8_t>>;

        void flush()
        {
            m_ser.adapter().flush();
            m_bytes.resize(m_ser.adapter().writtenBytesCount());
        }

        std::vector<uint8_t> m_bytes{};
        bitsery::Serializer<output_adapter> m_ser;
    };

    class deserializer : public serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const std::vector<uint8_t>& bytes)
            : m_bytes(bytes), m_ser(m_bytes.cbegin(), m_bytes.size())
        {
            EXTENSER_POSTCONDITION(!m_bytes.empty());
        }

        template<typename T>
        void as_bool([[maybe_unused]] const std::string_view key, T& val)
        {
            m_ser.value1b(val);
        }

        template<typename T>
        void as_float([[maybe_unused]] const std::string_view key, T& val)
        {
            static_assert(sizeof(T) <= sizeof(double), "long double not supported");
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_int([[maybe_unused]] const std::string_view key, T& val)
        {
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_string([[maybe_unused]] const std::string_view key, T& val)
        {
            m_ser.text1b(val, config::max_string_size);
        }

        template<typename T>
        void as_array([[maybe_unused]] const std::string_view key, T& val)
        {
            if constexpr (std::is_arithmetic_v<typename T::value_type>)
            {
                m_ser.container<sizeof(typename T::value_type)>(val, config::max_container_size);
            }
            else
            {
                m_ser.container(val, config::max_container_size);
            }
        }

        template<typename T, size_t N>
        void as_array([[maybe_unused]] const std::string_view key, span<T>& val)
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                m_ser.container<sizeof(T)>(val);
            }
            else
            {
                m_ser.container(val);
            }
        }

        template<typename T>
        void as_map([[maybe_unused]] const std::string_view key, T& val)
        {
            using S = bitsery::Deserializer<input_adapter>;
            using key_t = typename T::key_type;
            using val_t = typename T::mapped_type;

            m_ser.ext(val, bitsery::ext::StdMap{ config::max_container_size },
                [this](S& ser, key_t& map_key, val_t& map_val)
                {
                    serial_adapter::parse_obj(ser, *this, map_key);
                    serial_adapter::parse_obj(ser, *this, map_val);
                });
        }

        template<typename T>
        void as_multimap([[maybe_unused]] const std::string_view key, T& val)
        {
            as_map(key, val);
        }

        template<typename T1, typename T2>
        void as_tuple([[maybe_unused]] const std::string_view key, std::pair<T1, T2>& val)
        {
            serial_adapter::parse_obj(m_ser, *this, val.first);
            serial_adapter::parse_obj(m_ser, *this, val.second);
        }

        template<typename... Args>
        void as_tuple([[maybe_unused]] const std::string_view key, std::tuple<Args...>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            m_ser.ext(val,
                bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                    {
                        serial_adapter::parse_obj(ser, *this, subval);
                    } });
        }

        template<typename T>
        void as_optional([[maybe_unused]] const std::string_view key, std::optional<T>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            m_ser.ext(val, bitsery::ext::StdOptional{},
                [this](S& ser, T& subval) { serial_adapter::parse_obj(ser, *this, subval); });
        }

        template<typename... Args>
        void as_variant([[maybe_unused]] const std::string_view key, std::variant<Args...>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            m_ser.ext(val,
                bitsery::ext::StdVariant{ [this](S& ser, auto& subval)
                    {
                        serial_adapter::parse_obj(ser, *this, subval);
                    } });
        }

        template<typename T>
        void as_object([[maybe_unused]] const std::string_view key, T& val)
        {
            serial_adapter::parse_obj(m_ser, *this, val);
        }

        void as_null([[maybe_unused]] const std::string_view key)
        {
            // nop
        }

    private:
        using config = typename serial_adapter::config;
        using input_adapter = bitsery::InputBufferAdapter<std::vector<uint8_t>>;

        const std::vector<uint8_t>& m_bytes;
        bitsery::Deserializer<input_adapter> m_ser;
    };

    template<typename S, typename T, typename Adapter, bool Deserialize>
    void serial_adapter::parse_obj(S& ser, serializer_base<Adapter, Deserialize>& fallback, T& val)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            if constexpr (config::use_exact_size)
            {
                ser.template value<sizeof(T)>(val);
            }
            else
            {
                ser.value8b(val);
            }
        }
        else if constexpr (detail::is_stringlike_v<T>)
        {
            ser.text1b(val, config::max_string_size);
        }
        else if constexpr (detail::is_pair_v<T>)
        {
            parse_obj(ser, fallback, val.first);
            parse_obj(ser, fallback, val.second);
        }
        else if constexpr (detail::is_optional_v<T>)
        {
            using val_t = typename T::value_type;

            ser.ext(val, bitsery::ext::StdOptional{},
                [&fallback](S& s_ser, val_t& u_val) { parse_obj(s_ser, fallback, u_val); });
        }
        else if constexpr (detail::is_map_v<T>)
        {
            using key_t = typename T::key_type;
            using val_t = typename T::mapped_type;

            ser.ext(val, bitsery::ext::StdMap{ config::max_container_size },
                [&fallback](S& s_ser, key_t& map_key, val_t& map_val)
                {
                    parse_obj(s_ser, fallback, map_key);
                    parse_obj(s_ser, fallback, map_val);
                });
        }
        else if constexpr (detail::is_set_v<T>)
        {
            using key_t = typename T::key_type;

            ser.ext(val, bitsery::ext::StdSet{ config::max_container_size },
                [&fallback](S& s_ser, key_t& key_val) { parse_obj(s_ser, fallback, key_val); });
        }
        else if constexpr (detail::is_container_v<T>)
        {
            using val_t = typename T::value_type;

            if constexpr (std::is_arithmetic_v<val_t>)
            {
                if constexpr (bitsery::traits::ContainerTraits<std::remove_cv_t<T>>::isResizable)
                {
                    ser.template container<sizeof(val_t), std::remove_cv_t<T>>(
                        val, config::max_container_size);
                }
                else
                {
                    ser.template container<sizeof(val_t), std::remove_cv_t<T>>(val);
                }
            }
            else
            {
                if constexpr (bitsery::traits::ContainerTraits<std::remove_cv_t<T>>::isResizable)
                {
                    ser.container(val, config::max_container_size,
                        [](S& s_ser, std::string& substr)
                        { s_ser.text1b(substr, config::max_string_size); });
                }
                else
                {
                    ser.container(val,
                        [](S& s_ser, std::string& substr)
                        { s_ser.text1b(substr, config::max_string_size); });
                }
            }
        }
        else if constexpr (detail::is_tuple_v<T>)
        {
            ser.ext(val,
                bitsery::ext::StdTuple{ [&fallback](S& s_ser, auto& subval)
                    {
                        parse_obj(s_ser, fallback, subval);
                    } });
        }
        else if constexpr (detail::is_variant_v<T>)
        {
            ser.ext(val,
                bitsery::ext::StdVariant{ [&fallback](S& s_ser, auto& subval)
                    {
                        parse_obj(s_ser, fallback, subval);
                    } });
        }
        else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, std::monostate>
            || std::is_same_v<T, std::nullopt_t>)
        {
            // nop
        }
        else
        {
            serialize(fallback, val);
        }
    }
} //namespace detail_bitsery

using bitsery_adapter = detail_bitsery::serial_adapter;
} //namespace extenser
#endif //EXTENSER_BITSERY_HPP
