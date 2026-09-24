# Parallel branch benchmarks

Compared with `master`, this branch adds OpenMP work scheduling in the hierarchical geometry processor, compound region operations, and hierarchical connectivity. It also uses C++ parallel execution policies for sorting in edge processing, polygon tools, and netlist comparison when the standard library supports them. The DRC `threads` change exposes hierarchical cell parallelism through the existing command.

| Added path | Benchmark case |
| --- | --- |
| Hierarchical context and result processing | `hierarchical_and` at 0, 1, 2, and 4 configured threads |
| Compound region Boolean and generic child evaluation | `compound_bool_or` and `compound_interact` at 0, 2, and 4 configured threads |
| Hierarchical connection building | `hierarchical_connectivity` at 1, 2, and 4 OpenMP threads |
| Edge and polygon sorting | `edge_merge` and `polygon_rasterize` at two input sizes |
| Netlist comparison sorting | `netlist_compare` at two device counts |

Install Google Benchmark's development package with `benchmark.pc` and `pkg-config` (`libbenchmark-dev` on Ubuntu or `google-benchmark` with Homebrew). If using Homebrew's package with MacPorts `pkg-config`, set `PKG_CONFIG_PATH=$(brew --prefix)/lib/pkgconfig`. Build KLayout with `-with-benchmark` and `-with-openmp`. The benchmark executable is optional. For a headless Ubuntu build, use `./build.sh -qmake qmake6 -without-qt -noruby -nopython -nolibgit2 -with-openmp -with-benchmark -build build-release -bin bin-release`.

Run `./scripts/benchmark_parallelize.sh build-release 5 --benchmark_out=parallel.json --benchmark_out_format=json`. The second argument is the repetition count; further arguments go to Google Benchmark. The script exits with an error when any case reports a failed result. To compare two builds that both contain this benchmark suite, collect JSON from each on the same machine with the same compiler flags and run `uv run --script scripts/plot_parallel_benchmarks.py before=before.json after=after.json --output-dir benchmark-plots`. This writes `runtime.svg`, `thread-scaling.svg`, and `build-speedup.svg`. One input produces the first two plots.

These are end-to-end workloads, so their timings include work beyond the modified sort and task calls. Google Benchmark reports wall time and items per second. `dbParallelTests` checks exact hierarchical AND geometry and worker use in the regular unit test suite. C++ parallel sort falls back to serial on standard libraries without execution policy support.
