
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

#ifndef HDR_dbSIMDUtils
#define HDR_dbSIMDUtils

#include <cmath>

//  Portable, opt-in SIMD helpers with a scalar fallback.
//
//  These wrappers expose small, determinism-safe vector primitives (no change
//  in rounding versus the equivalent scalar IEEE-754 double operations) for the
//  hot geometry kernels. Where the compiler advertises SSE2 (every x86-64
//  target by default) the SSE2 intrinsics are used; otherwise the scalar
//  fallback is compiled, so the code builds and behaves identically on ARM,
//  MSVC without /arch, etc. SSE2 sqrt/arithmetic on doubles is IEEE-754 correct
//  and produces bit-identical results to the scalar path.

#if defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)))
#  define DB_HAVE_SSE2 1
#  include <emmintrin.h>
#endif

namespace db
{

/**
 *  @brief Computes sqrt(a) and sqrt(b) together, writing them to out[0] and out[1]
 *
 *  With SSE2 this is a single packed sqrt. The result is bit-identical to two
 *  scalar std::sqrt calls for every input: non-negative values yield the
 *  correctly-rounded root, negative values yield a quiet NaN (as std::sqrt
 *  does), so callers relying on NaN propagation behave identically.
 */
inline void simd_sqrt2 (double a, double b, double out[2])
{
#if defined(DB_HAVE_SSE2)
  __m128d v = _mm_set_pd (b, a);            //  [a, b]
  v = _mm_sqrt_pd (v);
  _mm_storeu_pd (out, v);
#else
  out[0] = std::sqrt (a);
  out[1] = std::sqrt (b);
#endif
}

}

#endif
