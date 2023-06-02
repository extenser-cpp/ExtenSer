// ExtenSer - An extensible, generic serialization library for C++
//
// Copyright (c) 2023 by Jackson Harmer
//
// SPDX-License-Identifier: BSD-3-Clause
// Distributed under The 3-Clause BSD License
// See accompanying file LICENSE or a copy at
// https://opensource.org/license/bsd-3-clause/

#ifndef EXTENSER_DETAIL_MACROS_HPP
#define EXTENSER_DETAIL_MACROS_HPP

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
#  include <cassert>
#  define EXTENSER_ASSERTION(EXPR) assert(EXPR)
#elif defined(EXTENSER_ASSERT_STDERR)
#  include <cstdio>
#  define EXTENSER_ASSERTION(EXPR)                                                               \
      if (!(EXPR))                                                                               \
      std::fprintf(stderr,                                                                       \
          "EXTENSER_ASSERTION: \"%s\" failed!\n  func: %s,\n  file: %s,\n  line: %d\n\n", #EXPR, \
          __FUNCTION__, __FILE__, __LINE__)
#elif defined(EXTENSER_ASSERT_THROW)
#  include <stdexcept>
#  define EXTENSER_ASSERTION(EXPR) \
      if (!(EXPR))                 \
      throw std::runtime_error("EXTENSER_ASSERTION: \"" #EXPR "\" failed!")
#elif defined(EXTENSER_ASSERT_ABORT)
#  include <cstdio>
#  define EXTENSER_ASSERTION(EXPR) \
      if (!(EXPR))                 \
      std::abort()
#elif defined(EXTENSER_ASSERT_ASSUME)
#  define EXTENSER_ASSERTION(EXPR) EXTENSER_ASSUME(EXPR)
#else
#  include <cassert>
#  define EXTENSER_ASSERTION(EXPR) assert(EXPR)
#endif

#if defined(EXTENSER_ASSERT_THROW)
inline constexpr bool EXTENSER_ASSERT_NOTHROW = false;
#else
inline constexpr bool EXTENSER_ASSERT_NOTHROW = true;
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

#endif
