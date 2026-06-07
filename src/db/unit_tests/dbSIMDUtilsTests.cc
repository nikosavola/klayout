
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

#include "dbSIMDUtils.h"
#include "tlUnitTest.h"

#include <cmath>
#include <vector>
#include <cstring>
#include <cstdint>

//  Whatever ISA path the build selects (scalar, SSE2, NEON, AVX2, AVX-512), the
//  result must be bit-identical to a scalar std::sqrt loop, including the quiet
//  NaN for negative inputs that callers (the DRC distance check) rely on.

static bool bit_same (double x, double y)
{
  if (std::isnan (x) && std::isnan (y)) {
    return true;
  }
  uint64_t a, b;
  std::memcpy (&a, &x, sizeof (a));
  std::memcpy (&b, &y, sizeof (b));
  return a == b;
}

//  simd_sqrt2: the 2-wide helper used by the Euclidian DRC check
TEST(1_sqrt2)
{
  double o[2];

  db::simd_sqrt2 (9.0, 16.0, o);
  EXPECT_EQ (o[0], 3.0);
  EXPECT_EQ (o[1], 4.0);

  db::simd_sqrt2 (0.0, 2.0, o);
  EXPECT_EQ (o[0], 0.0);
  EXPECT_EQ (o[1], std::sqrt (2.0));

  //  negative -> quiet NaN (matches std::sqrt, leaves caller's min/max unchanged)
  db::simd_sqrt2 (-1.0, 4.0, o);
  EXPECT_EQ (std::isnan (o[0]), true);
  EXPECT_EQ (o[1], 2.0);
}

//  simd_sqrt: the batch primitive (AVX-512 / AVX2 / SSE2 / NEON / scalar tiers)
TEST(2_sqrt_batch_all_lengths)
{
  std::vector<double> in;
  for (int i = 0; i < 1000; ++i) {
    in.push_back ((i % 7 == 0) ? -double (i) * 0.5 : double (i) * 3.14159 + 0.001);
  }
  in.push_back (-1e-18);
  in.push_back (0.0);
  in.push_back (-0.0);
  in.push_back (1e300);

  //  cover all lengths up to 71 so every chunk width (8/4/2/1) and remainder is exercised
  for (size_t n = 0; n <= 71 && n <= in.size (); ++n) {
    std::vector<double> out (n);
    db::simd_sqrt (in.data (), out.data (), n);
    for (size_t i = 0; i < n; ++i) {
      EXPECT_EQ (bit_same (out[i], std::sqrt (in[i])), true);
    }
  }
}

//  in == out aliasing must be supported
TEST(3_sqrt_batch_aliasing)
{
  std::vector<double> ref, buf;
  for (int i = 0; i < 50; ++i) {
    double v = double (i) * 1.5 + 0.25;
    ref.push_back (std::sqrt (v));
    buf.push_back (v);
  }
  db::simd_sqrt (buf.data (), buf.data (), buf.size ());
  for (size_t i = 0; i < buf.size (); ++i) {
    EXPECT_EQ (bit_same (buf[i], ref[i]), true);
  }
}
