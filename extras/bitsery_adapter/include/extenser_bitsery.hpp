// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_BITSERY_HPP
#define EXTENSER_BITSERY_HPP

#include <extenser/extenser.hpp>

#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_optional.h>
#include <bitsery/ext/std_set.h>
#include <bitsery/ext/std_tuple.h>
#include <bitsery/ext/std_variant.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/deque.h>
#include <bitsery/traits/forward_list.h>
#include <bitsery/traits/list.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace bitsery::traits
{
template<typename CharT, typename Traits>
struct ContainerTraits<std::basic_string_view<CharT, Traits>> :
    StdContainer<std::basic_string_view<CharT, Traits>, false, true>
{
};

#if defined(__cpp_lib_span)
template<typename T, std::size_t N>
struct ContainerTraits<std::span<T, N>> : StdContainer<std::span<T, N>, false, true>
{
};
#else
template<typename T>
struct ContainerTraits<extenser::span<T>> : StdContainer<extenser::span<T>, false, true>
{
};
#endif

template<typename CharT, typename Traits>
struct TextTraits<std::basic_string_view<CharT, Traits>>
{
    using TValue = typename ContainerTraits<std::basic_string_view<CharT, Traits>>::TValue;
    static constexpr bool addNUL = false;

    static size_t length(const std::basic_string_view<CharT, Traits>& str) { return str.size(); }
};
} //namespace bitsery::traits

namespace extenser
{
namespace detail_bitsery
{
    [[nodiscard]] inline auto make_bitsery_serialization_error(const error_code code,
        const std::string_view summary, const std::string_view operation,
        const std::string_view expected_type = {}, const std::string_view actual_type = {},
        const std::string_view detail_text = {}) -> serialization_error
    {
        return detail::make_serialization_error(
            code, summary, operation, {}, expected_type, actual_type, detail_text);
    }

    [[nodiscard]] inline auto bitsery_reader_error_name(const bitsery::ReaderError error)
        -> std::string_view
    {
        switch (error)
        {
            case bitsery::ReaderError::NoError:
                return "NoError";
            case bitsery::ReaderError::ReadingError:
                return "ReadingError";
            case bitsery::ReaderError::DataOverflow:
                return "DataOverflow";
            case bitsery::ReaderError::InvalidData:
                return "InvalidData";
            case bitsery::ReaderError::InvalidPointer:
                return "InvalidPointer";
            default:
                return "UnknownReaderError";
        }
    }

    [[nodiscard]] inline auto bitsery_error_code(const bitsery::ReaderError error) noexcept
        -> error_code
    {
        switch (error)
        {
            case bitsery::ReaderError::DataOverflow:
                return error_code::out_of_bounds;
            case bitsery::ReaderError::InvalidData:
            case bitsery::ReaderError::InvalidPointer:
            case bitsery::ReaderError::ReadingError:
                return error_code::parse_failure;
            case bitsery::ReaderError::NoError:
            default:
                return error_code::unknown;
        }
    }

    [[nodiscard]] inline auto make_bitsery_deserialization_error(const std::string_view operation,
        const bitsery::ReaderError error, const std::size_t pos) -> deserialization_error
    {
        std::string detail{};
        detail.append("reader_error=");
        detail.append(bitsery_reader_error_name(error));
        detail.append(", read_pos=");
        detail.append(std::to_string(pos));

        return detail::make_deserialization_error(bitsery_error_code(error),
            "Bitsery adapter reported an invalid binary input state", operation, {},
            "valid bitsery stream", bitsery_reader_error_name(error), detail);
    }

    template<typename CharT>
    [[nodiscard]] auto make_string_view_from_pointer(const CharT* str_ptr)
        -> std::basic_string_view<std::remove_cv_t<CharT>>
    {
        if (str_ptr == nullptr)
        {
            throw make_bitsery_serialization_error(error_code::invalid_argument,
                "Cannot serialize a null string pointer", "bitsery.serialize.string",
                "non-null string pointer", "null pointer");
        }

        return { str_ptr };
    }

    template<typename StringT>
    void validate_string_size(
        const StringT& value, const std::size_t max_size, const std::string_view operation)
    {
        const auto length = static_cast<std::size_t>(std::size(value));
        const auto serialized_size =
            length + (bitsery::traits::TextTraits<StringT>::addNUL ? 1U : 0U);

        if (serialized_size > max_size)
        {
            throw make_bitsery_serialization_error(error_code::invalid_size,
                "String exceeds the configured bitsery serialization limit", operation,
                std::to_string(max_size), std::to_string(serialized_size));
        }
    }

    template<typename ContainerT>
    void validate_container_size(
        const ContainerT& value, const std::size_t max_size, const std::string_view operation)
    {
        const auto container_size =
            static_cast<std::size_t>(std::distance(std::begin(value), std::end(value)));

        if (container_size > max_size)
        {
            throw make_bitsery_serialization_error(error_code::invalid_size,
                "Container exceeds the configured bitsery serialization limit", operation,
                std::to_string(max_size), std::to_string(container_size));
        }
    }

    class serializer;
    class deserializer;

    struct serial_adapter
    {
        using bytes_t = std::vector<std::uint8_t>;
        using serial_t = std::vector<std::uint8_t>;
        using serializer_t = serializer;
        using deserializer_t = deserializer;

        struct config
        {
            static const std::size_t max_string_size = 256;
            static const std::size_t max_container_size = 256;
        };

        template<typename S, typename T, typename Adapter, bool Deserialize>
        static void parse_obj(
            S& ser, detail::serializer_base<Adapter, Deserialize>& fallback, T& val);
    };

    class serializer : public detail::serializer_base<serial_adapter, false>
    {
    public:
        serializer() : m_ser(m_bytes) { m_bytes.reserve(64UL); }

        [[nodiscard]] auto object() & -> const std::vector<std::uint8_t>&
        {
            flush();
            return m_bytes;
        }

        [[nodiscard]] auto object() && -> std::vector<std::uint8_t>&&
        {
            flush();
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
            static_assert(sizeof(T) <= sizeof(double), "long double is not supported");
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_int([[maybe_unused]] const std::string_view key, const T& val)
        {
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_uint([[maybe_unused]] const std::string_view key, const T& val)
        {
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_enum([[maybe_unused]] const std::string_view key, const T val)
        {
            static_assert(std::is_enum_v<T>, "T must be an enum type");
            m_ser.value<sizeof(T)>(val);
        }

        template<typename T>
        void as_string([[maybe_unused]] const std::string_view key, const T& val)
        {
            using traits_t = containers::traits<T>;

            if constexpr (std::is_pointer_v<T>)
            {
                const auto tmp = make_string_view_from_pointer(val);
                m_ser.text<sizeof(std::remove_pointer_t<T>)>(tmp);
            }
            else if constexpr (std::is_array_v<T>)
            {
                m_ser.text<sizeof(std::remove_extent_t<T>)>(val);
            }
            else
            {
                static constexpr std::size_t char_sz = sizeof(typename traits_t::value_type);

                if constexpr (traits_t::has_fixed_size)
                {
                    m_ser.text<char_sz>(val);
                }
                else
                {
                    validate_string_size(val, config::max_string_size, "bitsery.serialize.string");
                    m_ser.text<char_sz>(val, config::max_string_size);
                }
            }
        }

        template<typename T>
        void as_array([[maybe_unused]] const std::string_view key, const T& val)
        {
            static_assert(!std::is_same_v<T, std::vector<bool>>, "Vector of bool not supported");

            using S = bitsery::Serializer<output_adapter>;
            using traits_t = containers::traits<T>;

            if constexpr (traits_t::is_sequential)
            {
                if constexpr (traits_t::has_fixed_size)
                {
                    if constexpr (std::is_arithmetic_v<typename traits_t::value_type>)
                    {
                        m_ser.container<sizeof(typename traits_t::value_type)>(val);
                    }
                    else
                    {
                        m_ser.container(val, [this](S& ser, typename traits_t::value_type& value)
                            { serial_adapter::parse_obj(ser, *this, value); });
                    }
                }
                else
                {
                    validate_container_size(
                        val, config::max_container_size, "bitsery.serialize.array");

                    if constexpr (std::is_arithmetic_v<typename traits_t::value_type>)
                    {
                        m_ser.container<sizeof(typename traits_t::value_type)>(
                            val, config::max_container_size);
                    }
                    else
                    {
                        m_ser.container(val, config::max_container_size,
                            [this](S& ser, typename traits_t::value_type& value)
                            { serial_adapter::parse_obj(ser, *this, value); });
                    }
                }
            }
            else
            {
                validate_container_size(val, config::max_container_size, "bitsery.serialize.array");
                m_ser.ext(val, bitsery::ext::StdSet{ config::max_container_size },
                    [this](S& ser, typename traits_t::value_type& value)
                    { serial_adapter::parse_obj(ser, *this, value); });
            }
        }

        template<typename T>
        void as_map([[maybe_unused]] const std::string_view key, const T& val)
        {
            using S = bitsery::Serializer<output_adapter>;
            using key_t = typename T::key_type;
            using val_t = typename T::mapped_type;

            validate_container_size(val, config::max_container_size, "bitsery.serialize.map");

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
            using S = bitsery::Serializer<output_adapter>;

            m_ser.object(val.first,
                [this](S& ser, T1& value) { serial_adapter::parse_obj(ser, *this, value); });
            m_ser.object(val.second,
                [this](S& ser, T2& value) { serial_adapter::parse_obj(ser, *this, value); });
        }

        template<typename... Args>
        void as_tuple([[maybe_unused]] const std::string_view key, const std::tuple<Args...>& val)
        {
            using S = bitsery::Serializer<output_adapter>;

            m_ser.ext(val,
                bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                    { serial_adapter::parse_obj(ser, *this, subval); } });
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
                    { serial_adapter::parse_obj(ser, *this, subval); } });
        }

        template<typename T>
        void as_object([[maybe_unused]] const std::string_view key, const T& val)
        {
            using S = bitsery::Serializer<output_adapter>;

            m_ser.object(
                val, [this](S& ser, T& value) { serial_adapter::parse_obj(ser, *this, value); });
        }

        void as_null([[maybe_unused]] const std::string_view key) noexcept
        {
            // nop
        }

    private:
        using config = serial_adapter::config;
        using output_adapter = bitsery::OutputBufferAdapter<std::vector<std::uint8_t>>;

        void flush()
        {
            m_ser.adapter().flush();
            m_bytes.resize(m_ser.adapter().writtenBytesCount());
        }

        std::vector<std::uint8_t> m_bytes{};
        bitsery::Serializer<output_adapter> m_ser;
    };

    class deserializer : public detail::serializer_base<serial_adapter, true>
    {
    public:
        explicit deserializer(const std::vector<std::uint8_t>& bytes) noexcept(
            EXTENSER_ASSERT_NOTHROW)
            : m_bytes(bytes), m_last_size(m_bytes.size()), m_ser(m_bytes.cbegin(), m_last_size)
        {
        }

        template<typename T>
        void as_bool([[maybe_unused]] const std::string_view key, T& val)
        {
            read_with_status("bitsery.deserialize.bool", [&]() { m_ser.value1b(val); });
        }

        template<typename T>
        void as_float([[maybe_unused]] const std::string_view key, T& val)
        {
            static_assert(sizeof(T) <= sizeof(double), "long double is not supported");
            read_with_status("bitsery.deserialize.float", [&]() { m_ser.value<sizeof(T)>(val); });
        }

        template<typename T>
        void as_int([[maybe_unused]] const std::string_view key, T& val)
        {
            read_with_status("bitsery.deserialize.int", [&]() { m_ser.value<sizeof(T)>(val); });
        }

        template<typename T>
        void as_uint([[maybe_unused]] const std::string_view key, T& val)
        {
            read_with_status("bitsery.deserialize.uint", [&]() { m_ser.value<sizeof(T)>(val); });
        }

        template<typename T>
        void as_enum([[maybe_unused]] const std::string_view key, T& val)
        {
            static_assert(std::is_enum_v<T>, "T must be an enum type");

            read_with_status("bitsery.deserialize.enum", [&]() { m_ser.value<sizeof(T)>(val); });
        }

        template<typename T>
        void as_string([[maybe_unused]] const std::string_view key, T& val)
        {
            using traits_t = containers::traits<T>;

            static constexpr std::size_t char_sz = sizeof(typename traits_t::value_type);

            read_with_status("bitsery.deserialize.string",
                [&]()
                {
                    if constexpr (traits_t::is_mutable)
                    {
                        if constexpr (traits_t::has_fixed_size)
                        {
                            m_ser.text<char_sz>(val);
                        }
                        else
                        {
                            m_ser.text<char_sz>(val, config::max_string_size);
                        }
                    }
                });
        }

        template<typename T>
        void as_array([[maybe_unused]] const std::string_view key, T& val)
        {
            static_assert(!std::is_same_v<T, std::vector<bool>>, "Vector of bool not supported");

            using traits_t = containers::traits<T>;
            using S = bitsery::Deserializer<input_adapter>;

            read_with_status("bitsery.deserialize.array",
                [&]()
                {
                    if constexpr (traits_t::is_mutable)
                    {
                        if constexpr (traits_t::is_sequential)
                        {
                            if constexpr (traits_t::has_fixed_size)
                            {
                                if constexpr (std::is_arithmetic_v<typename traits_t::value_type>)
                                {
                                    m_ser.container<sizeof(typename traits_t::value_type)>(val);
                                }
                                else
                                {
                                    m_ser.container(val,
                                        [this](S& ser, typename traits_t::value_type& value)
                                        { serial_adapter::parse_obj(ser, *this, value); });
                                }
                            }
                            else
                            {
                                if constexpr (std::is_arithmetic_v<typename traits_t::value_type>)
                                {
                                    m_ser.container<sizeof(typename traits_t::value_type)>(
                                        val, config::max_container_size);
                                }
                                else
                                {
                                    m_ser.container(val, config::max_container_size,
                                        [this](S& ser, typename traits_t::value_type& value)
                                        { serial_adapter::parse_obj(ser, *this, value); });
                                }
                            }
                        }
                        else
                        {
                            m_ser.ext(val, bitsery::ext::StdSet{ config::max_container_size },
                                [this](S& ser, typename traits_t::value_type& value)
                                { serial_adapter::parse_obj(ser, *this, value); });
                        }
                    }
                });
        }

        template<typename T>
        void as_map([[maybe_unused]] const std::string_view key, T& val)
        {
            using S = bitsery::Deserializer<input_adapter>;
            using key_t = typename T::key_type;
            using val_t = typename T::mapped_type;

            read_with_status("bitsery.deserialize.map",
                [&]()
                {
                    m_ser.ext(val, bitsery::ext::StdMap{ config::max_container_size },
                        [this](S& ser, key_t& map_key, val_t& map_val)
                        {
                            serial_adapter::parse_obj(ser, *this, map_key);
                            serial_adapter::parse_obj(ser, *this, map_val);
                        });
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
            read_with_status("bitsery.deserialize.pair",
                [&]()
                {
                    serial_adapter::parse_obj(m_ser, *this, val.first);
                    serial_adapter::parse_obj(m_ser, *this, val.second);
                });
        }

        template<typename... Args>
        void as_tuple([[maybe_unused]] const std::string_view key, std::tuple<Args...>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            read_with_status("bitsery.deserialize.tuple",
                [&]()
                {
                    m_ser.ext(val,
                        bitsery::ext::StdTuple{ [this](S& ser, auto& subval)
                            { serial_adapter::parse_obj(ser, *this, subval); } });
                });
        }

        template<typename T>
        void as_optional([[maybe_unused]] const std::string_view key, std::optional<T>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            read_with_status("bitsery.deserialize.optional",
                [&]()
                {
                    m_ser.ext(val, bitsery::ext::StdOptional{}, [this](S& ser, T& subval)
                        { serial_adapter::parse_obj(ser, *this, subval); });
                });
        }

        template<typename... Args>
        void as_variant([[maybe_unused]] const std::string_view key, std::variant<Args...>& val)
        {
            using S = bitsery::Deserializer<input_adapter>;

            read_with_status("bitsery.deserialize.variant",
                [&]()
                {
                    m_ser.ext(val,
                        bitsery::ext::StdVariant{ [this](S& ser, auto& subval)
                            { serial_adapter::parse_obj(ser, *this, subval); } });
                });
        }

        template<typename T>
        void as_object([[maybe_unused]] const std::string_view key, T& val)
        {
            read_with_status("bitsery.deserialize.object",
                [&]() { serial_adapter::parse_obj(m_ser, *this, val); });
        }

        void as_null([[maybe_unused]] const std::string_view key)
        {
            // nop
        }

    private:
        using config = serial_adapter::config;
        using input_adapter = bitsery::InputBufferAdapter<std::vector<std::uint8_t>>;

        template<typename Func>
        void read_with_status(const std::string_view operation, Func&& func)
        {
            update_buffer();
            std::forward<Func>(func)();
            ensure_reader_ok(operation);
        }

        void ensure_reader_ok(const std::string_view operation)
        {
            const auto error = m_ser.adapter().error();

            if (error != bitsery::ReaderError::NoError)
            {
                throw make_bitsery_deserialization_error(
                    operation, error, m_ser.adapter().currentReadPos());
            }
        }

        void update_buffer()
        {
            if (m_last_size != m_bytes.size())
            {
                m_last_size = m_bytes.size();
                const auto curPos = m_ser.adapter().currentReadPos();
                m_ser = bitsery::Deserializer<input_adapter>(m_bytes.cbegin(), m_last_size);
                m_ser.adapter().currentReadPos(curPos);
            }
        }

        const std::vector<std::uint8_t>& m_bytes;
        std::size_t m_last_size;
        bitsery::Deserializer<input_adapter> m_ser;
    };

    template<typename S, typename T, typename Adapter, bool Deserialize>
    void serial_adapter::parse_obj(
        S& ser, detail::serializer_base<Adapter, Deserialize>& fallback, T& val)
    {
        if constexpr (std::is_arithmetic_v<T>)
        {
            ser.template value<sizeof(T)>(val);
        }
        else if constexpr (detail::is_stringlike_v<T>)
        {
            if constexpr (std::is_pointer_v<T>)
            {
                const auto tmp = make_string_view_from_pointer(val);
                validate_string_size(tmp, config::max_string_size, "bitsery.serialize.string");
                ser.text1b(tmp, config::max_string_size);
            }
            else
            {
                validate_string_size(val, config::max_string_size, "bitsery.serialize.string");
                ser.text1b(val, config::max_string_size);
            }
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

            validate_container_size(val, config::max_container_size, "bitsery.serialize.map");

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

            validate_container_size(val, config::max_container_size, "bitsery.serialize.array");

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
                    validate_container_size(
                        val, config::max_container_size, "bitsery.serialize.array");
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
                    validate_container_size(
                        val, config::max_container_size, "bitsery.serialize.array");
                    ser.container(val, config::max_container_size, [](S& s_ser, std::string& substr)
                        { s_ser.text1b(substr, config::max_string_size); });
                }
                else
                {
                    ser.container(val, [](S& s_ser, std::string& substr)
                        { s_ser.text1b(substr, config::max_string_size); });
                }
            }
        }
        else if constexpr (detail::is_tuple_v<T>)
        {
            ser.ext(val,
                bitsery::ext::StdTuple{
                    [&fallback](S& s_ser, auto& subval) { parse_obj(s_ser, fallback, subval); } });
        }
        else if constexpr (detail::is_variant_v<T>)
        {
            ser.ext(val,
                bitsery::ext::StdVariant{
                    [&fallback](S& s_ser, auto& subval) { parse_obj(s_ser, fallback, subval); } });
        }
        else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, std::monostate>
            || std::is_same_v<T, std::nullopt_t>)
        {
            // nop
        }
        else
        {
            if constexpr (Deserialize)
            {
                fallback.deserialize_object(val);
            }
            else
            {
                fallback.serialize_object(val);
            }
        }
    }
} //namespace detail_bitsery

using bitsery_adapter = detail_bitsery::serial_adapter;
} //namespace extenser
#endif //EXTENSER_BITSERY_HPP
