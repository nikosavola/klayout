# Verification of the MPI (MPICH) tiling processor backend.
#
# This script is meant to be run under "mpiexec" with a varying number of
# ranks, e.g.:
#
#   mpiexec -n 1 klayout -b -r tiling_mpi_check.rb
#   mpiexec -n 2 klayout -b -r tiling_mpi_check.rb
#   mpiexec -n 4 klayout -b -r tiling_mpi_check.rb
#
# It builds a layout whose geometry spans many tiles (including a box that
# straddles tile boundaries) and runs several output channels of different
# types through the TilingProcessor: a Region (copy), an Edges collection and
# an EdgePairs collection (width check). The results are gathered onto rank 0.
#
# Two things are checked:
#
#  * The gathered, merged Region output must reproduce the full input geometry
#    exactly (a closed-form oracle, checked here on rank 0).
#  * The whole tiling plan is fixed regardless of the number of ranks - only the
#    rank a tile is assigned to changes - so every per-channel measure printed
#    on the "RESULT" line must be identical across rank counts. The harness that
#    drives this script (see the comment block) compares the RESULT line of
#    n=1 against n=2, n=4, ... to confirm the gather/replay path is correct for
#    every channel and output type.
#
# Output order is not deterministic across tiles/threads/ranks, so all measures
# are order-insensitive (area, bounding box, polygon/edge counts, total length).

rank = RBA::TilingProcessor::mpi_rank
size = RBA::TilingProcessor::mpi_size
$stderr.puts "rank #{rank} of #{size} (mpi_available=#{RBA::TilingProcessor::mpi_available?})"

ly = RBA::Layout::new
ly.dbu = 0.001
top = ly.create_cell("TOP")
l1 = ly.insert_layer(RBA::LayerInfo::new(1, 0))

# A 10x10 grid of small boxes (separated by gaps) plus one large box that
# straddles many tile boundaries, so the clip-and-merge path is exercised.
(0..9).each do |i|
  (0..9).each do |j|
    b = RBA::Box::new((i * 10 + 2) * 1000, (j * 10 + 2) * 1000,
                      (i * 10 + 9) * 1000, (j * 10 + 9) * 1000)
    top.shapes(l1).insert(b)
  end
end
top.shapes(l1).insert(RBA::Box::new(5000, 5000, 95000, 8000))

tp = RBA::TilingProcessor::new
tp.input("a", ly, top.cell_index, RBA::LayerInfo::new(1, 0))

region = RBA::Region::new       # channel "o"  - Region output
edges  = RBA::Edges::new        # channel "e"  - Edges output (non-Region type)
eps    = RBA::EdgePairs::new    # channel "ep" - EdgePairs output (non-Region type)
tp.output("o", region)
tp.output("e", edges)
tp.output("ep", eps)

tp.tile_size(10.0, 10.0)        # 10x10 um tiles over the ~100x100 um frame
tp.threads = 2                  # exercise the hybrid MPI + threads path
tp.queue("_output(o, a); _output(e, a.edges); _output(ep, a.width_check(10000))")
tp.execute("mpi tiling check")

# Only rank 0 holds the complete, gathered result.
if rank == 0

  region.merge

  # closed-form oracle: the merged copy must reproduce the full input geometry
  ref = RBA::Region::new(top.begin_shapes_rec(l1))
  ref.merge
  failures = []
  failures << "area: got #{region.area} expected #{ref.area}" if region.area != ref.area
  failures << "bbox: got #{region.bbox} expected #{ref.bbox}" if region.bbox.to_s != ref.bbox.to_s
  failures << "polygons: got #{region.count} expected #{ref.count}" if region.count != ref.count
  unless failures.empty?
    failures.each { |f| $stderr.puts "FAIL #{f}" }
    raise "MPI tiling verification FAILED with #{size} rank(s)"
  end

  # rank-count-invariant measures across all channels and output types
  puts "RESULT region_area=#{region.area} region_bbox=#{region.bbox} region_polys=#{region.count} " \
       "edges_len=#{edges.length} edges_n=#{edges.count} ep_n=#{eps.count}"
  $stderr.puts "PASS (ranks=#{size})"

end
