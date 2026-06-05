#!/bin/bash
#
# Minimal working example: run the KLayout MPI TilingProcessor benchmark on a
# Slurm cluster.
#
# Submit with:   sbatch slurm_benchmark.sbatch
#
# This requests one MPI rank per task and runs one KLayout process per rank; the
# tiling processor distributes the tiles across the ranks and gathers the result
# onto rank 0 (which is the only rank that prints the BENCH line).
#
#SBATCH --job-name=klayout-mpi-tiling
#SBATCH --nodes=2                 # number of compute nodes
#SBATCH --ntasks=8               # total MPI ranks (across all nodes)
#SBATCH --ntasks-per-node=4      # ranks per node
#SBATCH --cpus-per-task=1        # threads per rank; set >1 (and pass through
                                 # below) for the hybrid MPI + threads mode
#SBATCH --time=00:30:00
#SBATCH --output=klayout-mpi-%j.out

set -euo pipefail

# 1) Load the SAME MPI implementation KLayout was built against. KLayout here is
#    built against MPICH, so load MPICH (module name is cluster-specific). Using
#    a different stack (e.g. OpenMPI) makes every rank fall back to a singleton
#    world.
module load mpich 2>/dev/null || true

# 2) Point at the KLayout build on a filesystem visible to all nodes.
KLAYOUT_ROOT="${KLAYOUT_ROOT:-$HOME/klayout}"
export LD_LIBRARY_PATH="$KLAYOUT_ROOT/build-release:${LD_LIBRARY_PATH:-}"
KLAYOUT="$KLAYOUT_ROOT/build-release/klayout"
BENCH="$KLAYOUT_ROOT/testdata/mpi/benchmark"

# 3) Generate the benchmark layout once on the shared filesystem (so every node
#    reads the same file).
GDS="$BENCH/test_layout_many_cells.gds"
if [ ! -f "$GDS" ]; then
  PYTHONPATH="$KLAYOUT_ROOT/bin-release/pymod" \
    python3 "$BENCH/generate_layout.py" 1000 1000 "$GDS"
fi

# 4) Launch one KLayout per MPI rank. On Slurm, srun is the preferred launcher:
#    it reads --ntasks / --ntasks-per-node and wires up MPI through Slurm's PMI.
#    MPICH typically uses PMI-2 here; adjust --mpi to match your site
#    ("srun --mpi=list" shows the available plugins).
srun --mpi=pmi2 "$KLAYOUT" -b -r "$BENCH/tiling_benchmark.rb" \
     -rd gds="$GDS" -rd tile=250 -rd threads="${SLURM_CPUS_PER_TASK:-1}"

# Alternative, if srun's PMI is not set up for MPICH on your cluster, use
# MPICH's own Hydra launcher (it reads the Slurm node allocation automatically):
#
#   mpiexec -n "$SLURM_NTASKS" "$KLAYOUT" -b -r "$BENCH/tiling_benchmark.rb" \
#           -rd gds="$GDS" -rd tile=250 -rd threads="${SLURM_CPUS_PER_TASK:-1}"
