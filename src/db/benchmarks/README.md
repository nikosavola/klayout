# Parallel branch benchmarks

Compared with `master`, this branch adds OpenMP work scheduling in the hierarchical geometry processor, compound region operations, and hierarchical connectivity. It also uses C++ parallel execution policies for sorting in edge processing, polygon tools, netlist comparison, property-aware region merging, shape erasure, and PLC decomposition when the standard library supports them. The DRC `threads` change exposes hierarchical cell parallelism through the existing command.

| Added path | Benchmark case |
| --- | --- |
| Hierarchical context and result processing | `hierarchical_and` at 0, 1, 2, 3, 4, 6, 8, 10, and 12 configured threads |
| Compound region Boolean and generic child evaluation | `compound_bool_or` and `compound_interact` at 0, 1, 2, 3, 4, 6, 8, 10, and 12 configured threads |
| Hierarchical connection building | `hierarchical_connectivity` at 1, 2, 3, 4, 6, 8, 10, and 12 OpenMP threads |
| Edge and polygon sorting | `edge_merge` and `polygon_rasterize` at two input sizes |
| Netlist comparison sorting | `netlist_compare` at two device counts |
| Property-aware region merge sorting | `region_merge_properties` at two polygon counts |
| Shape erasure sorting | `shapes_erase` at two polygon counts |
| PLC decomposition sorting | `plc_decomposition` at two contour counts on Linux |

Install Google Benchmark's development package with `benchmark.pc` and `pkg-config` (`libbenchmark-dev` on Ubuntu or `google-benchmark` with Homebrew). If using Homebrew's package with MacPorts `pkg-config`, set `PKG_CONFIG_PATH=$(brew --prefix)/lib/pkgconfig`. Build KLayout with `-with-benchmark` and `-with-openmp`. The benchmark executable is optional. For a headless Ubuntu build, use `./build.sh -qmake qmake6 -without-qt -noruby -nopython -nolibgit2 -with-openmp -with-benchmark -build build-release -bin bin-release`.

Run `./scripts/benchmark_parallelize.sh build-release 5 --benchmark_out=parallel.json --benchmark_out_format=json`. The second argument is the repetition count; further arguments go to Google Benchmark. The script exits with an error when any case reports a failed result. To compare two builds that both contain this benchmark suite, collect JSON from each on the same machine with the same compiler flags and run `uv run --script scripts/plot_parallel_benchmarks.py before=before.json after=after.json --output-dir benchmark-plots`. This writes `runtime.svg`, `thread-scaling.svg`, and `build-speedup.svg`. One input produces the first two plots.

These are end-to-end workloads, so their timings include work beyond the modified sort and task calls. Google Benchmark reports wall time and items per second. The 12-thread point oversubscribes a 10-logical-CPU laptop and is shaded in the scaling plot. `dbParallelTests` checks exact hierarchical AND geometry and worker use in the regular unit test suite. The suite does not cover every changed sort site, and timings alone do not prove that a parallel backend ran. The PLC case is Linux-only because this contour hits a pre-existing PLC assertion on the macOS build. C++ parallel sort falls back to serial on standard libraries without execution policy support.
