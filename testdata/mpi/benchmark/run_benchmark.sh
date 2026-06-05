#!/usr/bin/env bash
#
# Driver for the MPI tiling processor benchmark.
#
# Generates a large layout (once), then runs the tiled boolean+sizing benchmark
# for a sweep of MPI rank counts and prints a speedup table. This is the MPI
# counterpart of the OpenMP/hierarchical benchmark - there the knob is the
# thread count, here it is the number of MPI ranks.
#
# Requirements:
#   * KLayout built with -with-mpi (so TilingProcessor distributes over ranks).
#   * MPICH's launcher. We prefer "mpiexec.mpich" so we do not accidentally use a
#     different MPI stack (e.g. OpenMPI) whose process manager klayout's MPICH
#     build cannot talk to.
#
# Usage:
#   ./run_benchmark.sh [ranks...]          # e.g. ./run_benchmark.sh 1 2 4 8
#
# Environment overrides:
#   KLAYOUT   path to the klayout binary (default: build-release/klayout)
#   LIBDIR    directory holding the klayout shared libs (default: build-release)
#   MPIEXEC   MPI launcher (default: mpiexec.mpich, else mpiexec)
#   TILE_UM   tile size in micron (default: 250)
#   THREADS   threads per rank (default: 1, i.e. pure MPI scaling)
#   CELLS,RECTS  layout size for the generator (defaults 1000, 1000)

set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../../.." && pwd)"

KLAYOUT="${KLAYOUT:-$root/build-release/klayout}"
LIBDIR="${LIBDIR:-$root/build-release}"
PYMOD="$root/bin-release/pymod"
TILE_UM="${TILE_UM:-250}"
THREADS="${THREADS:-1}"
CELLS="${CELLS:-1000}"
RECTS="${RECTS:-1000}"
GDS="$here/test_layout_many_cells.gds"

if command -v mpiexec.mpich >/dev/null 2>&1; then
  MPIEXEC="${MPIEXEC:-mpiexec.mpich}"
else
  MPIEXEC="${MPIEXEC:-mpiexec}"
fi

ranks=("$@")
if [ "${#ranks[@]}" -eq 0 ]; then
  ranks=(1 2 4 8)
fi

export LD_LIBRARY_PATH="$LIBDIR:${LD_LIBRARY_PATH:-}"

if [ ! -f "$GDS" ]; then
  echo "Generating benchmark layout ($CELLS cells x $RECTS rects) ..."
  python3 "$here/generate_layout.py" "$CELLS" "$RECTS" "$GDS"
fi

echo "Launcher: $MPIEXEC   KLayout: $KLAYOUT"
echo "Layout:   $GDS"
echo

base=""
printf "%-8s %-10s %-10s %-12s\n" "ranks" "exec_s" "speedup" "efficiency"
for n in "${ranks[@]}"; do
  line="$("$MPIEXEC" -n "$n" "$KLAYOUT" -b -r "$here/tiling_benchmark.rb" \
          -rd gds="$GDS" -rd tile="$TILE_UM" -rd threads="$THREADS" 2>/dev/null | grep '^BENCH' || true)"
  t="$(printf '%s\n' "$line" | sed -n 's/.*exec_s=\([0-9.]*\).*/\1/p')"
  if [ -z "$t" ]; then
    printf "%-8s %-10s\n" "$n" "FAILED"
    continue
  fi
  if [ -z "$base" ]; then base="$t"; fi
  sp="$(awk -v b="$base" -v t="$t" 'BEGIN{printf "%.2f", b/t}')"
  ef="$(awk -v b="$base" -v t="$t" -v n="$n" 'BEGIN{printf "%.0f%%", 100*(b/t)/n}')"
  printf "%-8s %-10s %-10s %-12s\n" "$n" "$t" "${sp}x" "$ef"
done
