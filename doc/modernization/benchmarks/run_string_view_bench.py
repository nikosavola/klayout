# /// script
# requires-python = ">=3.9"
# dependencies = []
# ///
"""
Phase 1.4 benchmark runner: const std::string & vs std::string_view.

Compiles doc/modernization/benchmarks/string_view_bench.cc with -O2 and times
the two modes. Prefers `hyperfine` (statistical, warmup) when it is on PATH and
falls back to a simple in-process timer otherwise.

Run with:

    uv run doc/modernization/benchmarks/run_string_view_bench.py
"""

import os
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "string_view_bench.cc")
ITERS = "20000"


def compile_bench(out_path: str) -> None:
    cxx = os.environ.get("CXX", "g++")
    cmd = [cxx, "-O2", "-std=c++17", SRC, "-o", out_path]
    print("compiling:", " ".join(cmd))
    subprocess.run(cmd, check=True)


def run_with_hyperfine(binary: str) -> None:
    cmd = [
        "hyperfine",
        "--warmup", "3",
        "-N",
        f"{binary} string {ITERS}",
        f"{binary} view {ITERS}",
    ]
    print("running:", " ".join(cmd))
    subprocess.run(cmd, check=True)


def run_with_timer(binary: str) -> None:
    def timeit(mode: str) -> float:
        best = float("inf")
        for _ in range(5):
            t0 = time.perf_counter()
            subprocess.run([binary, mode, ITERS], check=True,
                           stdout=subprocess.DEVNULL)
            best = min(best, time.perf_counter() - t0)
        return best

    t_string = timeit("string")
    t_view = timeit("view")
    print(f"\nbest-of-5 wall time ({ITERS} iterations):")
    print(f"  const std::string & : {t_string * 1e3:8.2f} ms")
    print(f"  std::string_view    : {t_view * 1e3:8.2f} ms")
    if t_view > 0:
        print(f"  speedup             : {t_string / t_view:8.2f}x")


def main() -> int:
    with tempfile.TemporaryDirectory() as d:
        binary = os.path.join(d, "string_view_bench")
        compile_bench(binary)
        if shutil.which("hyperfine"):
            run_with_hyperfine(binary)
        else:
            print("hyperfine not found; using built-in timer "
                  "(install hyperfine for statistical results)")
            run_with_timer(binary)
    return 0


if __name__ == "__main__":
    sys.exit(main())
