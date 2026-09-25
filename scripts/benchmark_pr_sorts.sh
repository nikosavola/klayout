#!/bin/bash

# Runs the PR 2365/2367 sort benchmarks against two builds and a range of CPU
# counts. Each build sits in its own directory with db_parallel_benchmarks and
# the klayout shared libraries (see scripts/plot_pr_sort_scaling.py for the
# plotting step). Runs after/before/after/before per CPU count so machine drift
# hits both builds equally. Layout: <bench-root>/{bench-before,bench-after}.

set -eu
cd "$(dirname -- "$0")/.."

bench_root=${1:-build-release/pr-bench}
cpu_list=${2:-"1 2 4 8 10"}
filter='^(region_merge_properties|shapes_erase|plc_decomposition)/'
bench_args=("--benchmark_filter=$filter" --benchmark_repetitions=5 --benchmark_min_time=0.5
  --benchmark_min_warmup_time=0.1 --benchmark_enable_random_interleaving --benchmark_out_format=json)

if [ ! -x "$bench_root/bench-after/db_parallel_benchmarks" ] || [ ! -x "$bench_root/bench-before/db_parallel_benchmarks" ]; then
  echo "Need $bench_root/bench-after and $bench_root/bench-before (binary plus klayout libraries)" >&2
  exit 1
fi

run () {
  build=$1; cpus=$2; pass=$3
  mask=0
  [ "$cpus" -gt 1 ] && mask="0-$((cpus - 1))"
  out="$bench_root/${build}-cpus${cpus}-pass${pass}.json"
  log="$bench_root/${build}-cpus${cpus}-pass${pass}.log"
  LD_LIBRARY_PATH="$bench_root/bench-$build" taskset -c "$mask" \
    "$bench_root/bench-$build/db_parallel_benchmarks" \
    "${bench_args[@]}" --benchmark_out="$out" >"$log" 2>&1
  if grep -q 'ERROR OCCURRED' "$log"; then
    echo "FAILED: $out" >&2
    exit 1
  fi
  echo "done $out"
}

for cpus in $cpu_list; do
  run after "$cpus" 1
  run before "$cpus" 1
  run before "$cpus" 2
  run after "$cpus" 2
done

echo "ALL DONE"