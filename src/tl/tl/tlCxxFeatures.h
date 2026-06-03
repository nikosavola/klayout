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
 *  the available C++ standard. It combines two sources of truth:
 *
 *    - The build-time HAVE_CPP20 / HAVE_CPP23 / HAVE_CPP26 defines that are
 *      activated from build.sh (see "-cpp20", "-cpp23", "-cpp26"). These are
 *      cumulative: enabling C++23 also defines HAVE_CPP20, etc.
 *
 *    - The standard feature-test macros (e.g. __cpp_lib_optional,
 *      __cpp_concepts, __cpp_lib_expected). These describe what the actual
 *      compiler/standard library supports, independent of the requested
 *      standard.
 *
 *  Modernization steps should prefer the TL_CXX* convenience macros below so
 *  that the default C++17 build keeps compiling unchanged and newer-standard
 *  features stay strictly opt-in.
 */

//  Provide __cpp_* feature-test macros where available (since C++20 these are
//  guaranteed to be predefined; <version> centralizes the library ones).
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

//  Language-standard level. __cplusplus is the most portable signal; the
//  HAVE_CPP* defines are an explicit override coming from the build system.
#if defined(HAVE_CPP20) || (defined(__cplusplus) && __cplusplus >= 202002L)
#  define TL_CXX20 1
#else
#  define TL_CXX20 0
#endif

#if defined(HAVE_CPP23) || (defined(__cplusplus) && __cplusplus >= 202302L)
#  define TL_CXX23 1
#else
#  define TL_CXX23 0
#endif

#if defined(HAVE_CPP26) || (defined(__cplusplus) && __cplusplus > 202302L)
#  define TL_CXX26 1
#else
#  define TL_CXX26 0
#endif

//  Library feature helpers (these stay 0 when the standard library does not
//  ship the corresponding header, even if the language level would allow it).
#if TL_CXX20 && defined(__cpp_concepts) && __cpp_concepts >= 201907L
#  define TL_HAS_CONCEPTS 1
#else
#  define TL_HAS_CONCEPTS 0
#endif

#if TL_CXX20 && defined(__cpp_lib_span)
#  define TL_HAS_SPAN 1
#else
#  define TL_HAS_SPAN 0
#endif

#if TL_CXX20 && defined(__cpp_lib_three_way_comparison)
#  define TL_HAS_SPACESHIP 1
#else
#  define TL_HAS_SPACESHIP 0
#endif

#if TL_CXX20 && defined(__cpp_lib_format)
#  define TL_HAS_STD_FORMAT 1
#else
#  define TL_HAS_STD_FORMAT 0
#endif

#if TL_CXX23 && defined(__cpp_lib_expected)
#  define TL_HAS_EXPECTED 1
#else
#  define TL_HAS_EXPECTED 0
#endif

#if TL_CXX23 && defined(__cpp_lib_mdspan)
#  define TL_HAS_MDSPAN 1
#else
#  define TL_HAS_MDSPAN 0
#endif

#if TL_CXX23 && defined(__cpp_lib_generator)
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

#endif
