#!/usr/bin/env python3
#
# Generates a large benchmark layout for the MPI tiling processor benchmark.
#
# It is based on the layout generator used for the OpenMP/hierarchical
# benchmark, but is laid out so the geometry is spread over a large 2D area:
# the tiling processor partitions that area into tiles and distributes the
# tiles across MPI ranks, so the work has to be spatially distributed to be
# parallelizable.
#
# Usage:
#   python3 generate_layout.py [cells] [rects] [out.gds]
#
# Defaults produce ~3M shapes (1000 cells x 1000 rects on two layers, plus a
# third overlapping box per rect), instanced on a grid covering ~2.5 x 4 mm.
#
# This uses the local pymod build if available; otherwise the installed
# "klayout" wheel.

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "..", "bin-release", "pymod"))
import klayout.db as db

cells = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
rects = int(sys.argv[2]) if len(sys.argv) > 2 else 1000
out = sys.argv[3] if len(sys.argv) > 3 else "test_layout_many_cells.gds"

layout = db.Layout()
l1 = layout.layer(1, 0)
l2 = layout.layer(2, 0)

top = layout.create_cell("TOP")
pitch = 100000  # 100 um in dbu
cols = 25       # grid columns; rows = ceil(cells / cols)

for i in range(cells):
    cell = layout.create_cell(f"CELL_{i}")
    for j in range(rects):
        # some randomish overlapping shapes
        x = (j * 10 % 100) * 1000
        y = (j * 15 % 100) * 1000
        cell.shapes(l1).insert(db.Box(x, y, x + 20000, y + 20000))
        cell.shapes(l2).insert(db.Box(x + 5000, y + 5000, x + 25000, y + 25000))
        cell.shapes(l1).insert(db.Box(x + 2000, y + 2000, x + 12000, y + 12000))

    # instance them all in TOP in a grid so the geometry spans a large area
    px = (i % cols) * pitch
    py = (i // cols) * pitch
    top.insert(db.CellInstArray(cell.cell_index(), db.Trans(db.Trans.R0, px, py)))

# flat noise to force the engine to process everything
for i in range(100):
    x = (i * 50) * 1000
    y = (i * 50) * 1000
    top.shapes(l2).insert(db.Box(x, y, x + 20000, y + 20000))

layout.write(out, db.SaveLayoutOptions())
print(f"Generated {out}: {cells} cells x {rects} rects, "
      f"top bbox {top.bbox().to_s()} (dbu)")
