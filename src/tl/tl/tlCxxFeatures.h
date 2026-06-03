/*

  KLayout Layout Viewer
  Copyright (C) 2006-2026 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef HDR_tlCxxFeatures_h
#define HDR_tlCxxFeatures_h

/**
 *  @file tlCxxFeatures.h
 *  @brief Central place for C++ standard / feature detection
 *
 *  This header gives the code base a single, consistent way to gate code on
 *  the available C++ standard. It infers everything from what the compiler
 *  actually offers - there is no separately injected HAVE_CPP* define:
 *
 *    - The language-standard level is read from __cplusplus (which reflects the
 *      -std= the compiler was invoked with). See TL_CXX20 / TL_CXX23 / TL_CXX26.
 *
 *    - Individual library/language features are detected via the standard
 *      feature-test macros (e.g. __cpp_concepts, __cpp_lib_expected,
 *      __cpp_lib_span). These describe exactly what the actual compiler /
 *      standard library supports. See the TL_HAS_* helpers.
 *
 *  Modernization steps should prefer the TL_CXX* / TL_HAS_* convenience macros
 *  below so that the default C++17 build keeps compiling unchanged and
 *  newer-standard features stay strictly opt-in (enabled simply by compiling
 *  against a newer -std=, e.g. CXXFLAGS=-std=c++20).
 */

//  Provide __cpp_* feature-test macros where available (since C++20 these are
//  guaranteed to be predefined; <version> centralizes the library ones).
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

//  Language-standard level, inferred from __cplusplus (the most portable signal;
//  it reflects the -std= the compiler was invoked with).
#if defined(__cplusplus) && __cplusplus >= 202002L
#  define TL_CXX20 1
#else
#  define TL_CXX20 0
#endif

//  Note on the threshold: a compiler in an in-progress mode reports an
//  intermediate __cplusplus value (e.g. GCC reports 202100L for -std=c++23
//  until the standard is finalized), so anything strictly greater than the
//  previous standard's value is treated as that standard.
#if defined(__cplusplus) && __cplusplus > 202002L
#  define TL_CXX23 1
#else
#  define TL_CXX23 0
#endif

#if defined(__cplusplus) && __cplusplus > 202302L
#  define TL_CXX26 1
#else
#  define TL_CXX26 0
#endif

//  Library feature helpers. These rely on the standard __cpp_lib_* / __cpp_*
//  feature-test macros, which are the authoritative signal that the header is
//  actually available. They stay 0 when the standard library does not ship the
//  feature, independently of the requested language level.
#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#  define TL_HAS_CONCEPTS 1
#else
#  define TL_HAS_CONCEPTS 0
#endif

#if defined(__cpp_lib_span)
#  define TL_HAS_SPAN 1
#else
#  define TL_HAS_SPAN 0
#endif

#if defined(__cpp_lib_three_way_comparison)
#  define TL_HAS_SPACESHIP 1
#else
#  define TL_HAS_SPACESHIP 0
#endif

#if defined(__cpp_lib_format)
#  define TL_HAS_STD_FORMAT 1
#else
#  define TL_HAS_STD_FORMAT 0
#endif

#if defined(__cpp_lib_expected)
#  define TL_HAS_EXPECTED 1
#else
#  define TL_HAS_EXPECTED 0
#endif

#if defined(__cpp_lib_mdspan)
#  define TL_HAS_MDSPAN 1
#else
#  define TL_HAS_MDSPAN 0
#endif

#if defined(__cpp_lib_generator)
#  define TL_HAS_GENERATOR 1
#else
#  define TL_HAS_GENERATOR 0
#endif

//  [[nodiscard]] is available in C++17, which is the project baseline (with
//  Qt6). For the C++11 fallback build (Qt5) it degrades to nothing.
#if defined(__cplusplus) && __cplusplus >= 201703L
#  define TL_NODISCARD [[nodiscard]]
#else
#  define TL_NODISCARD
#endif

//  std::string_view availability (C++17). The default Qt5 build still compiles
//  at C++11, so string_view is only exposed where the standard library provides
//  it. Read-only "view" parameters in hot paths (parsers, formatters) should be
//  written as:
//
//    #if TL_HAS_STRING_VIEW
//      void f (tl::string_view s);
//    #else
//      void f (const std::string &s);
//    #endif
//
//  so the C++11 build keeps the owning-reference overload.
#if defined(__cpp_lib_string_view) || (defined(__cplusplus) && __cplusplus >= 201703L)
#  define TL_HAS_STRING_VIEW 1
#  include <string_view>
namespace tl
{
  using string_view = std::string_view;
}
#else
#  define TL_HAS_STRING_VIEW 0
#endif

//  "if constexpr" enabler (Phase 1.5). Using "if constexpr" directly would break
//  the C++11 Qt5 build, so template code that wants compile-time branch pruning
//  writes TL_IF_CONSTEXPR instead:
//
//    template <class T>
//    void f (const T &x) {
//      TL_IF_CONSTEXPR (std::is_integral<T>::value) {
//        ... integer path ...
//      } else {
//        ... other path ...
//      }
//    }
//
//  On C++17+ this becomes "if constexpr" (the dead branch is discarded, enabling
//  SFINAE-free dispatch). On C++11/14 it degrades to an ordinary "if" - which is
//  exactly the pre-existing behaviour, so nothing regresses. Note: when relying
//  on a branch being *discarded* (e.g. it would not compile for some T), the
//  code must still also provide a C++11 fallback (tag dispatch / overload).
#if defined(__cpp_if_constexpr) || (defined(__cplusplus) && __cplusplus >= 201703L)
#  define TL_IF_CONSTEXPR if constexpr
#else
#  define TL_IF_CONSTEXPR if
#endif

//  std::span (C++20). Exposed as tl::span when the standard library provides it
//  (TL_HAS_SPAN). Use it to replace (pointer, size) parameter pairs in binary
//  readers, polygon point arrays and other buffer interfaces with a single
//  bounds-aware, zero-cost view:
//
//    #if TL_HAS_SPAN
//      void write (tl::span<const db::Point> points);
//    #else
//      void write (const db::Point *points, size_t n);
//    #endif
#if TL_HAS_SPAN
#  include <span>
namespace tl
{
  using std::span;
}
#endif

//  std::expected (C++23). Exposed as tl::expected / tl::unexpected when the
//  standard library provides it (TL_HAS_EXPECTED). This is the monadic,
//  exception-free alternative to the error-code/out-parameter patterns in file
//  I/O, parsing and network code:
//
//    #if TL_HAS_EXPECTED
//      tl::expected<Layout, std::string> read (const std::string &path);
//    #else
//      bool read (const std::string &path, Layout &out, std::string &error);
//    #endif
#if TL_HAS_EXPECTED
#  include <expected>
namespace tl
{
  using std::expected;
  using std::unexpected;
}
#endif

//  std::mdspan (C++23). Exposed as tl::mdspan / tl::extents / tl::dextents when
//  available (TL_HAS_MDSPAN). It is a non-owning, multi-dimensional view over a
//  flat buffer, ideal for the row/column rasterization grids in DRC and the
//  pixel rows in the image-processing layers:
//
//    #if TL_HAS_MDSPAN
//      tl::mdspan<uint8_t, tl::dextents<size_t, 2>> grid (data, rows, cols);
//      grid[r, c] = v;
//    #else
//      data[r * cols + c] = v;   //  manual index arithmetic
//    #endif
#if TL_HAS_MDSPAN
#  include <mdspan>
namespace tl
{
  using std::mdspan;
  using std::extents;
  using std::dextents;
}
#endif

//  std::generator (C++23). Exposed as tl::generator when available
//  (TL_HAS_GENERATOR). It is the standard coroutine generator for lazy
//  iteration, the intended replacement for the hand-written at_end()-style
//  iterator state machines (shape / cell traversal). See
//  doc/modernization/4.4-coroutines.md and doc/modernization/5.3-generator.md.
#if TL_HAS_GENERATOR
#  include <generator>
namespace tl
{
  using std::generator;
}
#endif

#endif
