#!/bin/sh

set -eu

repo_dir=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${1:-build-release}
repeats=${2:-5}
case "$build_dir" in
  /*) ;;
  *) build_dir="$repo_dir/$build_dir" ;;
esac

if [ ! -x "$build_dir/ut_runner" ]; then
  echo "No ut_runner at $build_dir/ut_runner" >&2
  exit 1
fi

library_path=
for dir in "$build_dir" "$build_dir"/* "$build_dir"/*/* "$build_dir"/*/*/*; do
  [ -d "$dir" ] || continue
  library_path=${library_path:+$library_path:}$dir
done

test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
export TESTSRC="$repo_dir" TESTTMP="$test_tmp"
export DYLD_LIBRARY_PATH="${DYLD_LIBRARY_PATH:+$DYLD_LIBRARY_PATH:}$library_path"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}$library_path"
"$build_dir/ut_runner" -ne -s -v "-r=$repeats" 'dbParallelBenchmarkTests:ParallelHierarchicalAndBenchmark'
