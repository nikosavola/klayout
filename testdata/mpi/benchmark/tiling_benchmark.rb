# MPI tiling processor benchmark.
#
# Runs a heavy, spatially-distributable tiling job (per-tile boolean AND of two
# layers plus a sizing operation) and times only TilingProcessor#execute. Under
# "mpiexec" the tiles are distributed across the ranks, so the execute time
# should drop as the rank count grows.
#
# This is the MPI counterpart of the OpenMP/hierarchical benchmark: there the
# parallelism knob is "dss.threads", here it is the number of MPI ranks (with an
# optional threads-per-rank for the hybrid case).
#
# Run via the klayout binary so MPI is initialized early, e.g.:
#   mpiexec.mpich -n 4 klayout -b -r tiling_benchmark.rb -rd gds=test_layout_many_cells.gds
#
# Optional run-time defines (-rd name=value):
#   gds=<file>          input layout (default test_layout_many_cells.gds)
#   tile=<um>           tile size in micron (default 250)
#   threads=<n>         threads per rank (default 1, i.e. pure MPI scaling)
#   size=<um>           sizing amount in micron (default 5)

gds     = $gds     || "test_layout_many_cells.gds"
tile_um = ($tile   || "250").to_f
threads = ($threads || "1").to_i
size_um = ($size   || "5").to_f

rank = RBA::TilingProcessor::mpi_rank
size = RBA::TilingProcessor::mpi_size

layout = RBA::Layout::new
layout.read(gds)
l1 = layout.find_layer(1, 0)
l2 = layout.find_layer(2, 0)
top = layout.top_cell

tp = RBA::TilingProcessor::new
tp.input("a", layout, top.cell_index, RBA::LayerInfo::new(1, 0))
tp.input("b", layout, top.cell_index, RBA::LayerInfo::new(2, 0))

r_and  = RBA::Region::new
r_size = RBA::Region::new
tp.output("o_and", r_and)
tp.output("o_size", r_size)

tp.tile_size(tile_um, tile_um)
tp.tile_border(size_um, size_um)   # so the sizing operation is correct across tile borders
tp.threads = threads
tp.queue("_output(o_and, a & b); _output(o_size, a.sized(#{(size_um * 1000).to_i}))")

t0 = Time.now
tp.execute("mpi benchmark")
dt = Time.now - t0

if rank == 0
  puts "BENCH ranks=#{size} threads=#{threads} tile_um=#{tile_um} " \
       "and_count=#{r_and.count} size_count=#{r_size.count} exec_s=#{format('%.3f', dt)}"
end
