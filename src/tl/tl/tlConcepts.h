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

#ifndef HDR_tlConcepts
#define HDR_tlConcepts

#include "tlCxxFeatures.h"

/**
 *  @file tlConcepts.h
 *  @brief C++20 concepts for constraining KLayout templates (Phase 4.1)
 *
 *  These concepts replace ad-hoc std::enable_if / SFINAE constraints with named,
 *  self-documenting requirements that produce far better error messages. They
 *  are only defined when the compiler supports concepts (TL_HAS_CONCEPTS, i.e.
 *  a C++20 build); the surrounding code must keep an unconstrained template for
 *  the C++11/17 default build.
 *
 *  Recommended usage pattern:
 *
 *    #if TL_HAS_CONCEPTS
 *    template <tl::Coordinate C>
 *    #else
 *    template <class C>
 *    #endif
 *    C clip (C v, C lo, C hi) { ... }
 */

#if TL_HAS_CONCEPTS

#include <concepts>
#include <type_traits>
#include <string>
#include <iterator>

namespace tl
{

/**
 *  @brief A coordinate type: an arithmetic value usable in geometry math.
 *
 *  KLayout coordinates are either integer (db::Coord) or floating point
 *  (db::DCoord). This excludes bool and character types, which are arithmetic
 *  but not meaningful coordinates.
 */
template <class T>
concept Coordinate =
  (std::integral<T> || std::floating_point<T>) &&
  !std::same_as<std::remove_cv_t<T>, bool> &&
  !std::same_as<std::remove_cv_t<T>, char>;

/**
 *  @brief An integer coordinate type (e.g. db::Coord).
 */
template <class T>
concept IntCoordinate = Coordinate<T> && std::integral<T>;

/**
 *  @brief A forward-iterable range (has begin()/end()).
 */
template <class T>
concept Iterable = requires (T &t) {
  { t.begin () };
  { t.end () };
  requires std::input_or_output_iterator<decltype (t.begin ())>;
};

/**
 *  @brief A type that can be serialized to / parsed from a tl string.
 *
 *  Matches the convention used across the code base: a to_string() producing a
 *  std::string. The parse side is intentionally not required here because it is
 *  usually a free function / extractor.
 */
template <class T>
concept Serializable = requires (const T &t) {
  { t.to_string () } -> std::convertible_to<std::string>;
};

/**
 *  @brief A shape type that exposes a bounding box.
 *
 *  Most db shape types (Polygon, Box, Edge, Path, Text) provide a box() method
 *  returning their bounding box. This concept captures that contract.
 */
template <class T>
concept BoundedShape = requires (const T &t) {
  { t.box () };
};

}

#endif  //  TL_HAS_CONCEPTS

#endif
