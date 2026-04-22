// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_DETAIL_ERROR_HPP
#define EXTENSER_DETAIL_ERROR_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace extenser
{
enum class error_code : std::uint8_t
{
    unknown = 0,
    assertion_failed,
    invalid_argument,
    missing_key,
    type_mismatch,
    invalid_enum_value,
    out_of_bounds,
    invalid_size,
    invalid_variant_index,
    argument_count_mismatch,
    parse_failure,
    external_failure
};

class extenser_exception : public std::runtime_error
{
public:
    explicit extenser_exception(const char* message, const error_code code = error_code::unknown,
        std::string operation = {}, std::string key = {}, std::string expected_type = {},
        std::string actual_type = {}, std::string detail = {})
        : std::runtime_error(message),
          m_code(code),
          m_operation(std::move(operation)),
          m_key(std::move(key)),
          m_expected_type(std::move(expected_type)),
          m_actual_type(std::move(actual_type)),
          m_detail(std::move(detail))
    {
    }

    explicit extenser_exception(const std::string& message,
        const error_code code = error_code::unknown, std::string operation = {},
        std::string key = {}, std::string expected_type = {}, std::string actual_type = {},
        std::string detail = {})
        : std::runtime_error(message),
          m_code(code),
          m_operation(std::move(operation)),
          m_key(std::move(key)),
          m_expected_type(std::move(expected_type)),
          m_actual_type(std::move(actual_type)),
          m_detail(std::move(detail))
    {
    }

    [[nodiscard]] auto code() const noexcept -> error_code { return m_code; }
    [[nodiscard]] auto operation() const noexcept -> std::string_view { return m_operation; }
    [[nodiscard]] auto key() const noexcept -> std::string_view { return m_key; }
    [[nodiscard]] auto expected_type() const noexcept -> std::string_view
    {
        return m_expected_type;
    }

    [[nodiscard]] auto actual_type() const noexcept -> std::string_view { return m_actual_type; }
    [[nodiscard]] auto detail() const noexcept -> std::string_view { return m_detail; }

private:
    error_code m_code;
    std::string m_operation;
    std::string m_key;
    std::string m_expected_type;
    std::string m_actual_type;
    std::string m_detail;
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

class contract_violation : public extenser_exception
{
public:
    using extenser_exception::extenser_exception;
};

namespace detail
{
    [[nodiscard]] inline auto error_code_name(const error_code code) noexcept -> std::string_view
    {
        switch (code)
        {
            case error_code::unknown:
                return "unknown";
            case error_code::assertion_failed:
                return "assertion_failed";
            case error_code::invalid_argument:
                return "invalid_argument";
            case error_code::missing_key:
                return "missing_key";
            case error_code::type_mismatch:
                return "type_mismatch";
            case error_code::invalid_enum_value:
                return "invalid_enum_value";
            case error_code::out_of_bounds:
                return "out_of_bounds";
            case error_code::invalid_size:
                return "invalid_size";
            case error_code::invalid_variant_index:
                return "invalid_variant_index";
            case error_code::argument_count_mismatch:
                return "argument_count_mismatch";
            case error_code::parse_failure:
                return "parse_failure";
            case error_code::external_failure:
                return "external_failure";
            default:
                return "unknown";
        }
    }

    inline void append_message_field(
        std::string& message, const std::string_view label, const std::string_view value)
    {
        if (value.empty())
        {
            return;
        }

        message.append(" [");
        message.append(label.data(), label.size());
        message.append(": ");
        message.append(value.data(), value.size());
        message.push_back(']');
    }

    [[nodiscard]] inline auto build_error_message(const std::string_view category,
        const std::string_view summary, const error_code code,
        const std::string_view operation = {}, const std::string_view key = {},
        const std::string_view expected_type = {}, const std::string_view actual_type = {},
        const std::string_view detail = {}) -> std::string
    {
        std::string message{};

        if (!category.empty())
        {
            message.append(category.data(), category.size());
        }

        if (!summary.empty())
        {
            if (!message.empty())
            {
                message.append(": ");
            }

            message.append(summary.data(), summary.size());
        }
        else if (!message.empty())
        {
            message.append(": ");
            const auto code_name = error_code_name(code);
            message.append(code_name.data(), code_name.size());
        }

        append_message_field(message, "code", error_code_name(code));
        append_message_field(message, "operation", operation);
        append_message_field(message, "key", key);
        append_message_field(message, "expected", expected_type);
        append_message_field(message, "actual", actual_type);
        append_message_field(message, "detail", detail);

        return message;
    }

    [[nodiscard]] inline auto make_serialization_error(const error_code code,
        const std::string_view summary, const std::string_view operation = {},
        const std::string_view key = {}, const std::string_view expected_type = {},
        const std::string_view actual_type = {}, const std::string_view detail = {})
        -> serialization_error
    {
        return serialization_error{ build_error_message("Serialization error", summary, code,
                                        operation, key, expected_type, actual_type, detail),
            code, std::string{ operation }, std::string{ key }, std::string{ expected_type },
            std::string{ actual_type }, std::string{ detail } };
    }

    [[nodiscard]] inline auto make_deserialization_error(const error_code code,
        const std::string_view summary, const std::string_view operation = {},
        const std::string_view key = {}, const std::string_view expected_type = {},
        const std::string_view actual_type = {}, const std::string_view detail = {})
        -> deserialization_error
    {
        return deserialization_error{ build_error_message("Deserialization error", summary, code,
                                          operation, key, expected_type, actual_type, detail),
            code, std::string{ operation }, std::string{ key }, std::string{ expected_type },
            std::string{ actual_type }, std::string{ detail } };
    }

    [[nodiscard]] inline auto make_contract_violation(const std::string_view expression,
        const std::string_view function_name, const std::string_view file_name, const int line)
        -> contract_violation
    {
        std::string detail{};
        detail.append("expression=");
        detail.append(expression.data(), expression.size());
        detail.append(", file=");
        detail.append(file_name.data(), file_name.size());
        detail.append(", line=");
        detail.append(std::to_string(line));

        return contract_violation{ build_error_message("Contract violation",
                                       "ExtenSer precondition or postcondition failed",
                                       error_code::assertion_failed, function_name, {}, {}, {},
                                       detail),
            error_code::assertion_failed, std::string{ function_name }, {}, {}, {},
            std::move(detail) };
    }
} //namespace detail
} //namespace extenser

#endif