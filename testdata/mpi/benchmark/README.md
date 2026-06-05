# MPI tiling processor benchmark

Measures the speedup of the MPI (MPICH) tiling-processor backend as the number
of ranks grows. It is the MPI counterpart of the OpenMP/hierarchical benchmark:
there the parallelism knob is the thread count, here it is the number of MPI
ranks (with an optional threads-per-rank for the hybrid case).

## Files

- `generate_layout.py` — generates a large layout (default 1000 cells ×
  1000 rects, ~3M shapes, instanced on a ~5 × 5 mm grid). Uses the local
  `bin-release/pymod` build.
- `tiling_benchmark.rb` — runs a per-tile boolean `AND` plus a sizing operation
  through the `TilingProcessor` and times only `execute()` (each rank loads the
  GDS first; that load is redundant SPMD overhead and is not timed).
- `run_benchmark.sh` — generates the layout once, sweeps the rank counts and
  prints a speedup table.

## Running

Build KLayout with MPI first:

```
./build.sh -with-mpi -option -j8
```

Then run the sweep (prefer MPICH's launcher explicitly, see the caveat below):

```
testdata/mpi/benchmark/run_benchmark.sh 1 2 4 8
```

## On a Slurm cluster

`slurm_benchmark.sbatch` is a minimal working example. Submit it with:

```
sbatch slurm_benchmark.sbatch
```

It requests one MPI rank per Slurm task and launches one KLayout per rank with
`srun` (the preferred Slurm launcher, which wires up MPI through Slurm's PMI).
Set `KLAYOUT_ROOT` to a build visible on all nodes, and load the same MPI module
KLayout was built against. For the hybrid MPI + threads mode, raise
`--cpus-per-task`; it is passed through as the per-rank thread count. Only rank 0
prints the `BENCH` line in the job output.

## Example result

8-core machine (Intel), pure MPI scaling (`threads=1`, 250 µm tiles over the
~5 × 5 mm layout):

| ranks | exec_s | speedup | efficiency |
|------:|-------:|--------:|-----------:|
| 1     | 46.13  | 1.00×   | 100%       |
| 2     | 23.10  | 2.00×   | 100%       |
| 4     | 11.68  | 3.95×   | 99%        |
| 8     | 10.52  | 4.38×   | 55%        |

Near-linear up to 4 ranks; the 8-rank point tapers because the box has only
8 cores (no spare core for the OS/IO, plus the redundant per-rank GDS load and
memory-bandwidth contention). `and_count`/`size_count` are identical across all
rank counts, which doubles as a correctness check on the gather/replay path.

## Using MPI from the Python API / a wheel

The MPI distribution lives in the C++ `TilingProcessor` (in `libklayout_db`),
so the Python module gets it for free **if the module was built with MPI**
(`klayout.db.TilingProcessor.mpi_available()` returns `True`). The pymod built
by `build.sh -with-mpi` is MPI-enabled; the public PyPI `klayout` wheel is not.

Unlike the GUI binary, Python does not need the early `MPI_Init` hook: the
Python interpreter does not clobber the launcher's environment before the
tiling processor lazily initializes MPI, so a plain launch works:

```
mpiexec.mpich -n 4 python3 my_tiling_script.py
```

Inside the script, guard output handling with the rank, e.g.:

```python
import klayout.db as db
tp = db.TilingProcessor()
# ... configure inputs/outputs/tiles, then:
tp.execute("job")
if db.TilingProcessor.mpi_rank() == 0:
    region.write("result.gds")   # only rank 0 holds the complete result
```

### Caveats (one consistent MPI stack)

A single process must use exactly one MPI implementation. In practice:

- Launch with the **same** MPI that KLayout was linked against. KLayout here is
  built against MPICH, so use `mpiexec.mpich`. Installing an OpenMPI-based
  package (e.g. Debian's `python3-mpi4py`) can switch the default `mpiexec`
  alternative to OpenMPI, whose process manager the MPICH build cannot talk to —
  every rank then falls back to a singleton world (`mpi_size() == 1`).
- If you also use `mpi4py`, it must be built against the **same** MPICH. A
  mismatched `mpi4py` (e.g. OpenMPI) initializes a different MPI library in the
  process and will not share KLayout's `MPI_COMM_WORLD`.
