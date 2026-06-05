
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

#ifndef HDR_dbScanlineCompare
#define HDR_dbScanlineCompare

#include "dbEdge.h"

#include <cstdint>
#include <algorithm>

#if defined(HAVE_64BIT_COORD) || !defined(__SIZEOF_INT128__)
#  include "tlLongInt.h"
#endif

namespace db
{

/**
 *  @brief A wide signed integer type used for the exact scanline x comparison
 *
 *  The scanline algorithm has to order edges by the x coordinate at which they
 *  cross the current scanline (see db::edge_xaty). Historically that x value was
 *  computed as a "double" and compared with the help of the "volatile" keyword
 *  to force a deterministic, reproducible rounding. The "volatile" trick defeats
 *  register allocation, vectorization and instruction-level parallelism in the
 *  innermost comparator of the algorithm.
 *
 *  Instead we compare the x positions *exactly* using rational arithmetic:
 *  edge_xaty (e, y) equals num/den (with den > 0). Comparing num_a/den_a against
 *  num_b/den_b reduces to comparing the cross products num_a*den_b and
 *  num_b*den_a. The numerator can use the full 64 bit range and the cross
 *  product therefore needs up to ~98 bits, so a 128 bit integer is required.
 *
 *  This is exact and deterministic on every platform without any reliance on
 *  floating point behaviour, so no "volatile" is needed. Both the __int128 and
 *  the tl::long_int paths compute the identical mathematical result, preserving
 *  cross-platform determinism.
 *
 *  The width of the integer type has to be chosen according to the coordinate
 *  type. For 32 bit coordinates the cross product needs up to ~98 bits, so 128
 *  bits suffice: __int128 where available (gcc, clang, MinGW) and KLayout's
 *  portable emulation otherwise (e.g. MSVC). For 64 bit coordinates
 *  (HAVE_64BIT_COORD) the numerator alone reaches ~129 bits and the cross
 *  product ~194 bits, so a 256 bit integer is required. The static_assert below
 *  guards against an accidentally too-small type so this can never silently
 *  overflow into wrong orderings.
 */
#if defined(HAVE_64BIT_COORD)
typedef tl::long_int<8, uint32_t, uint64_t> scanline_wide_int;    //  256 bit
#elif defined(__SIZEOF_INT128__)
typedef __int128 scanline_wide_int;                               //  128 bit
#else
typedef tl::long_int<4, uint32_t, uint64_t> scanline_wide_int;    //  128 bit
#endif

//  The cross product num_a*den_b spans roughly 3*sizeof(Coord) bytes worth of
//  bits; require at least 4*sizeof(Coord) bytes of headroom.
static_assert (sizeof (scanline_wide_int) >= 4 * sizeof (db::Coord),
               "scanline_wide_int is too narrow for the coordinate type");

/**
 *  @brief The exact x position of an edge at a scanline, as a rational num/den
 *
 *  The denominator is always strictly positive. Both num and den are kept in
 *  the wide integer type so that all intermediate coordinate differences are
 *  computed without overflow, regardless of the coordinate width.
 */
struct ScanlineX
{
  scanline_wide_int num;
  scanline_wide_int den;
};

/**
 *  @brief Exact equivalent of db::edge_xaty for the scanline comparison
 *
 *  Returns the x position of edge e at scanline y as an exact rational num/den
 *  (den > 0). The real value num/den is identical to edge_xaty (e, y).
 */
inline ScanlineX scanline_xaty (db::Edge e, db::Coord y)
{
  if (e.p1 ().y () > e.p2 ().y ()) {
    e.swap_points ();
  }

  ScanlineX r;
  if (y <= e.p1 ().y ()) {
    r.num = scanline_wide_int (e.p1 ().x ());
    r.den = scanline_wide_int (1);
  } else if (y >= e.p2 ().y ()) {
    r.num = scanline_wide_int (e.p2 ().x ());
    r.den = scanline_wide_int (1);
  } else {
    scanline_wide_int p1x (e.p1 ().x ());
    scanline_wide_int dy = scanline_wide_int (e.p2 ().y ()) - scanline_wide_int (e.p1 ().y ());   //  > 0
    scanline_wide_int dx = scanline_wide_int (e.p2 ().x ()) - scanline_wide_int (e.p1 ().x ());
    scanline_wide_int dyy = scanline_wide_int (y) - scanline_wide_int (e.p1 ().y ());
    r.num = p1x * dy + dx * dyy;
    r.den = dy;
  }
  return r;
}

/**
 *  @brief Exact equivalent of edge_xaty2 (delivers the minimum x for horizontal edges)
 */
inline ScanlineX scanline_xaty2 (db::Edge e, db::Coord y)
{
  if (e.p1 ().y () > e.p2 ().y ()) {
    e.swap_points ();
  }

  ScanlineX r;
  if (y <= e.p1 ().y ()) {
    if (y == e.p2 ().y ()) {
      r.num = scanline_wide_int (std::min (e.p1 ().x (), e.p2 ().x ()));
    } else {
      r.num = scanline_wide_int (e.p1 ().x ());
    }
    r.den = scanline_wide_int (1);
  } else if (y >= e.p2 ().y ()) {
    r.num = scanline_wide_int (e.p2 ().x ());
    r.den = scanline_wide_int (1);
  } else {
    scanline_wide_int p1x (e.p1 ().x ());
    scanline_wide_int dy = scanline_wide_int (e.p2 ().y ()) - scanline_wide_int (e.p1 ().y ());   //  > 0
    scanline_wide_int dx = scanline_wide_int (e.p2 ().x ()) - scanline_wide_int (e.p1 ().x ());
    scanline_wide_int dyy = scanline_wide_int (y) - scanline_wide_int (e.p1 ().y ());
    r.num = p1x * dy + dx * dyy;
    r.den = dy;
  }
  return r;
}

/**
 *  @brief Exact three-way comparison of two scanline x positions
 *
 *  Returns -1 if a < b, +1 if a > b and 0 if a == b. Because both denominators
 *  are positive, sign (a - b) == sign (a.num*b.den - b.num*a.den).
 */
inline int scanline_x_compare (const ScanlineX &a, const ScanlineX &b)
{
  scanline_wide_int lhs = a.num * b.den;
  scanline_wide_int rhs = b.num * a.den;
  if (lhs < rhs) {
    return -1;
  } else if (rhs < lhs) {
    return 1;
  } else {
    return 0;
  }
}

}

#endif
