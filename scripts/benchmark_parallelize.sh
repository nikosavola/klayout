#!/bin/sh

set -eu

repo_dir=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${1:-build-release}
repeats=${2:-5}
case "$build_dir" in
  /*) ;;
  *) build_dir="$repo_dir/$build_dir" ;;
esac

if [ ! -x "$build_dir/db_parallel_benchmarks" ]; then
  echo "No db_parallel_benchmarks at $build_dir/db_parallel_benchmarks; build with -with-benchmark" >&2
  exit 1
fi

library_path=
for dir in "$build_dir" "$build_dir"/* "$build_dir"/*/* "$build_dir"/*/*/*; do
  [ -d "$dir" ] || continue
  library_path=${library_path:+$library_path:}$dir
done

export DYLD_LIBRARY_PATH="${DYLD_LIBRARY_PATH:+$DYLD_LIBRARY_PATH:}$library_path"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}$library_path"
shift "$(( $# < 2 ? $# : 2 ))"
"$build_dir/db_parallel_benchmarks" "--benchmark_repetitions=$repeats" --benchmark_min_time=0.5s --benchmark_min_warmup_time=0.1 --benchmark_enable_random_interleaving "$@"
