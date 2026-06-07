
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
#include <cstddef>

//  Portable, opt-in SIMD helpers with a scalar fallback.
//
//  These wrappers expose small, determinism-safe vector primitives for the hot
//  geometry kernels. They are guarded by the compiler's ISA feature macros, so
//  the widest implementation available for the *build target* is selected and
//  everything else (including non-SIMD targets) falls back to portable scalar
//  code. IEEE-754 double sqrt and arithmetic are correctly rounded on all of
//  these instruction sets, and a negative input yields a quiet NaN exactly as
//  std::sqrt does, so every path produces bit-identical results - which is why
//  callers that rely on NaN propagation (e.g. the DRC distance check) stay
//  deterministic regardless of which path is compiled.
//
//  Note on widths: KLayout's libraries are normally built for the SSE2 (x86-64)
//  baseline, so only the SSE2 / scalar paths are active in a default build. The
//  AVX2 / AVX-512 paths below are compiled only when the build enables those
//  features (e.g. -mavx2 / -mavx512f / -march=native), and the NEON path only
//  on AArch64. They are "present but dormant" until then.

#if defined(__AVX512F__)
#  define DB_HAVE_AVX512F 1
#endif
#if defined(__AVX2__)
#  define DB_HAVE_AVX2 1
#endif
#if defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)))
#  define DB_HAVE_SSE2 1
#endif
//  AArch64 always provides the Advanced SIMD (NEON) double-precision sqrt.
#if defined(__aarch64__) || defined(_M_ARM64)
#  define DB_HAVE_NEON64 1
#endif

#if defined(DB_HAVE_AVX512F) || defined(DB_HAVE_AVX2)
#  include <immintrin.h>
#elif defined(DB_HAVE_SSE2)
#  include <emmintrin.h>
#endif
#if defined(DB_HAVE_NEON64)
#  include <arm_neon.h>
#endif

namespace db
{

/**
 *  @brief Computes sqrt(a) and sqrt(b) together, writing them to out[0] and out[1]
 *
 *  Two doubles fill a 128-bit vector register, so the SSE2 (x86) and NEON
 *  (AArch64) packed sqrt are the optimal implementations here. AVX2 / AVX-512
 *  operate on 4 / 8 doubles and therefore cannot accelerate a two-element sqrt -
 *  see simd_sqrt() for the batch primitive that uses those wider instructions.
 *
 *  The result is bit-identical to two scalar std::sqrt calls for every input:
 *  non-negative values yield the correctly-rounded root, negative values yield
 *  a quiet NaN, so callers relying on NaN propagation behave identically.
 */
inline void simd_sqrt2 (double a, double b, double out[2])
{
#if defined(DB_HAVE_SSE2)
  _mm_storeu_pd (out, _mm_sqrt_pd (_mm_set_pd (b, a)));   //  lanes [a, b]
#elif defined(DB_HAVE_NEON64)
  double in[2] = { a, b };
  vst1q_f64 (out, vsqrtq_f64 (vld1q_f64 (in)));
#else
  out[0] = std::sqrt (a);
  out[1] = std::sqrt (b);
#endif
}

/**
 *  @brief Batched square root: out[i] = sqrt(in[i]) for i in [0, n)
 *
 *  This primitive processes the array with the widest vector the build target
 *  supports - AVX-512 (8 doubles per step), AVX2/AVX (4), SSE2 or NEON (2) -
 *  and a scalar remainder, falling back entirely to scalar where no SIMD is
 *  available. in and out may alias (in == out is supported). Negative inputs
 *  produce NaN, matching std::sqrt, so the output is bit-identical to a scalar
 *  loop on every instruction set.
 */
inline void simd_sqrt (const double *in, double *out, std::size_t n)
{
  std::size_t i = 0;

#if defined(DB_HAVE_AVX512F)
  for ( ; i + 8 <= n; i += 8) {
    _mm512_storeu_pd (out + i, _mm512_sqrt_pd (_mm512_loadu_pd (in + i)));
  }
#endif
#if defined(DB_HAVE_AVX2)
  for ( ; i + 4 <= n; i += 4) {
    _mm256_storeu_pd (out + i, _mm256_sqrt_pd (_mm256_loadu_pd (in + i)));
  }
#endif
#if defined(DB_HAVE_SSE2)
  for ( ; i + 2 <= n; i += 2) {
    _mm_storeu_pd (out + i, _mm_sqrt_pd (_mm_loadu_pd (in + i)));
  }
#elif defined(DB_HAVE_NEON64)
  for ( ; i + 2 <= n; i += 2) {
    vst1q_f64 (out + i, vsqrtq_f64 (vld1q_f64 (in + i)));
  }
#endif

  for ( ; i < n; ++i) {
    out[i] = std::sqrt (in[i]);
  }
}

}

#endif
