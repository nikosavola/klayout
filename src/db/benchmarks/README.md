# Parallel geometry benchmarks

Install Google Benchmark's development package with `benchmark.pc` and `pkg-config` (`libbenchmark-dev` on Ubuntu or `google-benchmark` with Homebrew). If using Homebrew's package with MacPorts `pkg-config`, set `PKG_CONFIG_PATH=$(brew --prefix)/lib/pkgconfig`. Build KLayout with `-with-benchmark`; add `-with-openmp` to exercise the OpenMP paths. The benchmark executable is optional and does not affect normal builds. For a headless Ubuntu build, use `./build.sh -qmake qmake6 -without-qt -noruby -nopython -nolibgit2 -with-openmp -with-benchmark -build build-release -bin bin-release`.

Run `./scripts/benchmark_parallelize.sh build-release 5` from the repository root. The second argument is the number of repetitions. Further arguments go to Google Benchmark, such as `--benchmark_filter=hierarchical_and` or `--benchmark_out=results.json`.

The suite measures hierarchical AND at 0, 1, 2, and 4 processing threads, plus edge merge at two geometry sizes. It reports wall time and items per second. Compare JSON results from two builds on the same machine; run each build with the same flags and release settings. The regular `dbParallelTests` unit test checks exact geometry and worker use.
