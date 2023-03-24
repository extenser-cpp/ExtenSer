// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_SPAN_HPP
#define EXTENSER_SPAN_HPP

#include "detail/macros.hpp"
#include "detail/type_traits.hpp"

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

#if defined(__cpp_lib_span)
#  include <span>
#endif

namespace extenser
{
#if defined(__cpp_lib_span)
using std::as_bytes;
using std::as_writable_bytes;
using std::span;
#else
namespace detail
{
    template<typename T>
    class span_iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cv_t<T>;
        using pointer = T*;
        using reference = T&;

        constexpr span_iterator(pointer ptr) noexcept : m_ptr(ptr) {}

        constexpr auto operator*() const noexcept -> reference { return *m_ptr; }
        constexpr auto operator->() const noexcept -> pointer { return m_ptr; }

        constexpr auto operator++() noexcept -> span_iterator&
        {
            ++m_ptr;
            return *this;
        }

        constexpr auto operator++(int) noexcept -> span_iterator
        {
            span_iterator tmp{ *this };
            ++*this;
            return tmp;
        }

        constexpr auto operator--() noexcept -> span_iterator&
        {
            --m_ptr;
            return *this;
        }

        constexpr auto operator--(int) -> span_iterator
        {
            span_iterator tmp{ *this };
            --*this;
            return tmp;
        }

        constexpr auto operator+=(const difference_type offset) noexcept -> span_iterator&
        {
            m_ptr += offset;
            return *this;
        }

        friend constexpr auto operator+(
            const span_iterator& lhs, const difference_type offset) noexcept -> span_iterator
        {
            span_iterator tmp{ lhs };
            tmp += offset;
            return tmp;
        }

        friend constexpr auto operator+(const difference_type offset, span_iterator next) noexcept
            -> span_iterator
        {
            next += offset;
            return next;
        }

        constexpr auto operator-=(const difference_type offset) noexcept -> span_iterator&
        {
            m_ptr -= offset;
            return *this;
        }

        friend constexpr auto operator-(
            const span_iterator& lhs, const difference_type offset) noexcept -> span_iterator
        {
            span_iterator tmp{ lhs };
            tmp -= offset;
            return tmp;
        }

        friend constexpr auto operator-(const span_iterator& lhs, const span_iterator& rhs) noexcept
            -> difference_type
        {
            return lhs.m_ptr - rhs.m_ptr;
        }

        constexpr auto operator[](const difference_type offset) const noexcept -> reference
        {
            return *(*this + offset);
        }

        friend constexpr auto operator==(
            const span_iterator& lhs, const span_iterator& rhs) noexcept -> bool
        {
            return lhs.m_ptr == rhs.m_ptr;
        }

        friend constexpr auto operator!=(
            const span_iterator& lhs, const span_iterator& rhs) noexcept -> bool
        {
            return lhs.m_ptr != rhs.m_ptr;
        }

        constexpr auto to_address() const noexcept -> pointer { return m_ptr; }

    private:
        pointer m_ptr{ nullptr };
    };
} //namespace detail

// backport of C++20's span
template<typename T>
class span
{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = detail::span_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    ~span() noexcept = default;

    constexpr span() noexcept = default;

    template<typename It,
        typename = std::enable_if_t<detail::constructible_from_iterator_v<element_type, It>>>
    constexpr span(It first, size_type count) : m_head_ptr(&*first), m_sz(count)
    {
    }

    template<typename It, typename End,
        typename = std::enable_if_t<detail::constructible_from_iterator_v<element_type, It>
            && !std::is_convertible_v<End, size_type>>>
    constexpr span(It first, End last)
        : m_head_ptr(&*first), m_sz(static_cast<size_type>(std::distance(first, last)))
    {
        EXTENSER_PRECONDITION(std::distance(first, last) >= 0);
    }

    template<std::size_t N>
    constexpr span(detail::type_identity_t<element_type> (&arr)[N]) noexcept
        : m_head_ptr(std::data(arr)), m_sz(N)
    {
    }

    template<typename U, std::size_t N>
    constexpr span(std::array<U, N>& arr) noexcept : m_head_ptr(std::data(arr)), m_sz(N)
    {
    }

    template<typename U, std::size_t N>
    constexpr span(const std::array<U, N>& arr) noexcept : m_head_ptr(std::data(arr)), m_sz(N)
    {
    }

    template<typename U>
    constexpr span(const span<U>& source) noexcept
        : m_head_ptr(source.m_head_ptr), m_sz(source.m_sz)
    {
    }

    constexpr span(const span&) noexcept = default;
    constexpr span(span&&) = delete;

    constexpr auto operator=(const span&) noexcept -> span& = default;
    constexpr auto operator=(span&&) -> span& = delete;

    constexpr auto begin() const noexcept -> iterator { return { m_head_ptr }; }
    constexpr auto end() const noexcept -> iterator { return begin() + m_sz; }
    constexpr auto rbegin() const noexcept -> reverse_iterator { return reverse_iterator{ end() }; }
    constexpr auto rend() const noexcept -> reverse_iterator { return reverse_iterator{ begin() }; }

    constexpr auto front() const -> reference
    {
        EXTENSER_PRECONDITION(m_sz != 0);
        return m_head_ptr[0];
    }

    constexpr auto back() const -> reference
    {
        EXTENSER_PRECONDITION(m_sz != 0);
        return m_head_ptr[m_sz];
    }

    constexpr auto operator[](size_type idx) const -> reference
    {
        EXTENSER_PRECONDITION(idx < m_sz);
        return *(begin() + idx);
    }

    constexpr auto data() const noexcept -> pointer { return m_head_ptr; }
    [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_sz == 0; }
    constexpr auto size() const noexcept -> size_type { return m_sz; }
    constexpr auto size_bytes() const noexcept -> size_type { return m_sz * sizeof(T); }

    constexpr auto first(size_type count) const -> span
    {
        EXTENSER_PRECONDITION(count <= m_sz);
        return { begin(), count };
    }

    constexpr auto last(size_type count) const -> span
    {
        EXTENSER_PRECONDITION(count <= m_sz);
        return { end() - count, count };
    }

    constexpr auto subspan(size_type offset) const -> span
    {
        EXTENSER_PRECONDITION(offset <= m_sz);
        return { begin() + m_sz, end() };
    }

    constexpr auto subspan(size_type offset, size_type count) const -> span
    {
        EXTENSER_PRECONDITION(offset <= m_sz);
        EXTENSER_PRECONDITION(count <= m_sz - offset);
        return { begin() + offset, count };
    }

private:
    pointer m_head_ptr{ nullptr };
    size_type m_sz{ 0 };
};

template<typename T>
auto as_bytes(span<T> span) noexcept -> extenser::span<const std::byte>
{
    return { reinterpret_cast<const std::byte*>(span.data()), span.size_bytes() };
}

template<typename T>
auto as_writable_bytes(span<T> span) noexcept -> extenser::span<std::byte>
{
    return { reinterpret_cast<std::byte*>(span.data()), span.size_bytes() };
}

// Deduction guides
template<typename It, typename End>
span(It, End) -> span<std::remove_reference_t<decltype(*std::declval<It&>())>>;

template<typename T, std::size_t N>
span(T (&)[N]) -> span<T>;

template<typename T, std::size_t N>
span(std::array<T, N>&) -> span<T>;

template<typename T, std::size_t N>
span(const std::array<T, N>&) -> span<const T>;
#endif

template<typename T>
using view = span<const T>;
} //namespace extenser
#endif
