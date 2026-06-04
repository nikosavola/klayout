#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
KLayout GPU vs CPU Benchmarking Suite

This script benchmarks KLayout's GPU-accelerated operations against their
CPU counterparts. It measures performance for:

  1. Polygon Sizing (offset/inflate/deflate)
  2. Point-in-Polygon batch testing
  3. Batch Bounding Box computation

Usage:
    python gpu_benchmark.py [OPTIONS]

Options:
    --operations OP [OP ...]   Operations to benchmark (sizing, pip, bbox, all)
    --sizes N [N ...]          Polygon/point counts to test
    --iterations N             Number of iterations per measurement (default: 5)
    --warmup N                 Number of warmup iterations (default: 2)
    --output FILE              Output results to JSON file
    --csv FILE                 Output results to CSV file
    --no-plot                  Disable matplotlib plotting
    --verbose                  Print detailed progress information

Requirements:
    - klayout Python module (pip install klayout)
    - Optional: matplotlib (for plotting results)
    - Optional: numpy (for statistical analysis)

The script automatically detects whether GPU acceleration is available and
runs both CPU and GPU paths for comparison when possible. If no GPU is
available, only CPU benchmarks are run.
"""

import argparse
import json
import math
import os
import random
import statistics
import sys
import time
from collections import defaultdict
from contextlib import contextmanager

try:
    import klayout.db as db
except ImportError:
    print("Error: klayout Python module not found.", file=sys.stderr)
    print("Install with: pip install klayout", file=sys.stderr)
    sys.exit(1)


# ---------------------------------------------------------------------------
#  Utility helpers
# ---------------------------------------------------------------------------

@contextmanager
def timer():
    """Context manager that measures elapsed wall-clock time in seconds."""
    start = time.perf_counter()
    result = {"elapsed": 0.0}
    yield result
    result["elapsed"] = time.perf_counter() - start


def format_time(seconds):
    """Format a time duration for human-readable display."""
    if seconds < 1e-6:
        return f"{seconds * 1e9:.1f} ns"
    elif seconds < 1e-3:
        return f"{seconds * 1e6:.1f} µs"
    elif seconds < 1.0:
        return f"{seconds * 1e3:.1f} ms"
    else:
        return f"{seconds:.3f} s"


def format_count(n):
    """Format a count with K/M suffixes."""
    if n >= 1_000_000:
        return f"{n / 1_000_000:.1f}M"
    elif n >= 1_000:
        return f"{n / 1_000:.0f}K"
    return str(n)


# ---------------------------------------------------------------------------
#  Test data generation
# ---------------------------------------------------------------------------

def generate_random_polygon(num_vertices=8, bbox_size=10000):
    """Generate a random convex-ish polygon with the given number of vertices."""
    cx, cy = random.randint(0, bbox_size), random.randint(0, bbox_size)
    radius = random.randint(100, bbox_size // 4)

    points = []
    for i in range(num_vertices):
        angle = 2.0 * math.pi * i / num_vertices
        # Add some randomness to the radius
        r = radius * (0.7 + 0.6 * random.random())
        x = cx + int(r * math.cos(angle))
        y = cy + int(r * math.sin(angle))
        points.append(db.Point(x, y))

    return db.Polygon(points)


def generate_polygon_set(count, vertices_per_polygon=8, bbox_size=100000):
    """Generate a set of random polygons for benchmarking."""
    return [generate_random_polygon(vertices_per_polygon, bbox_size)
            for _ in range(count)]


def generate_random_points(count, bbox_size=100000):
    """Generate random points within a bounding box."""
    return [db.Point(random.randint(0, bbox_size), random.randint(0, bbox_size))
            for _ in range(count)]


def generate_region(polygon_count, vertices_per_polygon=8, bbox_size=100000):
    """Generate a Region containing random polygons."""
    region = db.Region()
    for _ in range(polygon_count):
        region.insert(generate_random_polygon(vertices_per_polygon, bbox_size))
    return region


# ---------------------------------------------------------------------------
#  GPU availability detection
# ---------------------------------------------------------------------------

def detect_gpu_info():
    """
    Detect GPU availability through KLayout's backend.

    Returns a dict with GPU information or None if unavailable.
    """
    info = {
        "available": False,
        "backend": "none",
        "device_name": "N/A",
    }

    # Check if the GPU backend class is exposed to Python
    if hasattr(db, "GPUBackend"):
        try:
            backend = db.GPUBackend.instance()
            info["available"] = backend.is_available()
            if info["available"]:
                dev_info = backend.device_info()
                info["backend"] = "CUDA" if backend.type() == 1 else "OpenCL"
                info["device_name"] = dev_info.name
                info["total_memory"] = dev_info.total_memory
                info["compute_capability"] = (
                    f"{dev_info.compute_capability_major}."
                    f"{dev_info.compute_capability_minor}"
                )
        except Exception:
            pass

    return info


# ---------------------------------------------------------------------------
#  Benchmark runner
# ---------------------------------------------------------------------------

class BenchmarkResult:
    """Stores the result of a single benchmark measurement."""

    def __init__(self, operation, backend, size, times):
        self.operation = operation
        self.backend = backend  # "cpu" or "gpu"
        self.size = size  # e.g., polygon count or point count
        self.times = times  # list of measured times in seconds
        self.mean = statistics.mean(times)
        self.median = statistics.median(times)
        self.stdev = statistics.stdev(times) if len(times) > 1 else 0.0
        self.min_time = min(times)
        self.max_time = max(times)

    def to_dict(self):
        return {
            "operation": self.operation,
            "backend": self.backend,
            "size": self.size,
            "mean_seconds": self.mean,
            "median_seconds": self.median,
            "stdev_seconds": self.stdev,
            "min_seconds": self.min_time,
            "max_seconds": self.max_time,
            "iterations": len(self.times),
        }


def run_benchmark(func, iterations=5, warmup=2):
    """
    Run a benchmark function multiple times and collect timing data.

    Args:
        func: Callable to benchmark (no arguments)
        iterations: Number of timed iterations
        warmup: Number of warmup iterations (not timed)

    Returns:
        List of elapsed times in seconds
    """
    # Warmup
    for _ in range(warmup):
        func()

    # Timed runs
    times = []
    for _ in range(iterations):
        with timer() as t:
            func()
        times.append(t["elapsed"])

    return times


# ---------------------------------------------------------------------------
#  Benchmark: Polygon Sizing
# ---------------------------------------------------------------------------

def benchmark_sizing(polygon_counts, iterations=5, warmup=2, verbose=False):
    """
    Benchmark polygon sizing operation (CPU vs GPU).

    Sizing (also called biasing or offset) grows or shrinks polygons by a
    specified amount. This is a common DRC operation and is embarrassingly
    parallel, making it ideal for GPU acceleration.
    """
    results = []

    for count in polygon_counts:
        if verbose:
            print(f"  Sizing: {format_count(count)} polygons...", flush=True)

        region = generate_region(count)
        dx, dy = 100, 100  # Sizing amount in database units

        # CPU benchmark - force CPU by disabling GPU
        def run_cpu():
            if hasattr(db, "GPUBackend"):
                db.GPUBackend.set_enabled(False)
            try:
                _ = region.sized(dx, dy, 2)  # mode 2 = octagon
            finally:
                if hasattr(db, "GPUBackend"):
                    db.GPUBackend.set_enabled(True)

        times_cpu = run_benchmark(run_cpu, iterations, warmup)
        results.append(BenchmarkResult("sizing", "cpu", count, times_cpu))

        if verbose:
            print(f"    CPU: {format_time(statistics.mean(times_cpu))} "
                  f"(±{format_time(statistics.stdev(times_cpu) if len(times_cpu) > 1 else 0)})")

        # GPU benchmark (if available)
        if hasattr(db, "GPUBackend"):
            def run_gpu():
                db.GPUBackend.set_enabled(True)
                _ = region.sized(dx, dy, 2)

            times_gpu = run_benchmark(run_gpu, iterations, warmup)
            results.append(BenchmarkResult("sizing", "gpu", count, times_gpu))

            if verbose:
                speedup = statistics.mean(times_cpu) / statistics.mean(times_gpu)
                print(f"    GPU: {format_time(statistics.mean(times_gpu))} "
                      f"(±{format_time(statistics.stdev(times_gpu) if len(times_gpu) > 1 else 0)}) "
                      f"[{speedup:.2f}x]")

    return results


# ---------------------------------------------------------------------------
#  Benchmark: Point-in-Polygon
# ---------------------------------------------------------------------------

def benchmark_point_in_polygon(point_counts, iterations=5, warmup=2, verbose=False):
    """
    Benchmark batch point-in-polygon testing (CPU vs GPU).

    Tests many points against a polygon to determine inside/outside/boundary
    status. This is a massively parallel operation where GPU excels because
    each point test is independent.
    """
    results = []

    # Create a complex test polygon (many vertices for realistic workload)
    test_polygon = generate_random_polygon(num_vertices=64, bbox_size=50000)
    test_region = db.Region(test_polygon)

    for count in point_counts:
        if verbose:
            print(f"  Point-in-polygon: {format_count(count)} points...", flush=True)

        # Generate test points spread around the polygon
        points = generate_random_points(count, bbox_size=50000)

        # Use Region.interacting with individual point regions as a proxy
        # for batch point-in-polygon testing
        point_region = db.Region()
        for p in points:
            # Create tiny boxes around each point
            point_region.insert(db.Box(p.x, p.y, p.x + 1, p.y + 1))

        # CPU benchmark
        def run_cpu():
            if hasattr(db, "GPUBackend"):
                db.GPUBackend.set_enabled(False)
            try:
                _ = test_region.interacting(point_region)
            finally:
                if hasattr(db, "GPUBackend"):
                    db.GPUBackend.set_enabled(True)

        times_cpu = run_benchmark(run_cpu, iterations, warmup)
        results.append(BenchmarkResult("point_in_polygon", "cpu", count, times_cpu))

        if verbose:
            print(f"    CPU: {format_time(statistics.mean(times_cpu))} "
                  f"(±{format_time(statistics.stdev(times_cpu) if len(times_cpu) > 1 else 0)})")

        # GPU benchmark
        if hasattr(db, "GPUBackend"):
            def run_gpu():
                db.GPUBackend.set_enabled(True)
                _ = test_region.interacting(point_region)

            times_gpu = run_benchmark(run_gpu, iterations, warmup)
            results.append(BenchmarkResult("point_in_polygon", "gpu", count, times_gpu))

            if verbose:
                speedup = statistics.mean(times_cpu) / statistics.mean(times_gpu)
                print(f"    GPU: {format_time(statistics.mean(times_gpu))} "
                      f"(±{format_time(statistics.stdev(times_gpu) if len(times_gpu) > 1 else 0)}) "
                      f"[{speedup:.2f}x]")

    return results


# ---------------------------------------------------------------------------
#  Benchmark: Bounding Box Computation
# ---------------------------------------------------------------------------

def benchmark_bounding_boxes(polygon_counts, iterations=5, warmup=2, verbose=False):
    """
    Benchmark batch bounding box computation (CPU vs GPU).

    Computing bounding boxes for many polygons in parallel. Each polygon's
    bounding box is independent, making this embarrassingly parallel.
    """
    results = []

    for count in polygon_counts:
        if verbose:
            print(f"  Bounding boxes: {format_count(count)} polygons...", flush=True)

        # Use polygons with varying vertex counts for realistic workload
        region = generate_region(count, vertices_per_polygon=16)

        # CPU benchmark using Region.bbox (aggregated) and per-polygon via each
        def run_cpu():
            if hasattr(db, "GPUBackend"):
                db.GPUBackend.set_enabled(False)
            try:
                # Force evaluation by iterating
                for poly in region.each():
                    _ = poly.bbox()
            finally:
                if hasattr(db, "GPUBackend"):
                    db.GPUBackend.set_enabled(True)

        times_cpu = run_benchmark(run_cpu, iterations, warmup)
        results.append(BenchmarkResult("bounding_boxes", "cpu", count, times_cpu))

        if verbose:
            print(f"    CPU: {format_time(statistics.mean(times_cpu))} "
                  f"(±{format_time(statistics.stdev(times_cpu) if len(times_cpu) > 1 else 0)})")

        # GPU benchmark
        if hasattr(db, "GPUBackend"):
            def run_gpu():
                db.GPUBackend.set_enabled(True)
                for poly in region.each():
                    _ = poly.bbox()

            times_gpu = run_benchmark(run_gpu, iterations, warmup)
            results.append(BenchmarkResult("bounding_boxes", "gpu", count, times_gpu))

            if verbose:
                speedup = statistics.mean(times_cpu) / statistics.mean(times_gpu)
                print(f"    GPU: {format_time(statistics.mean(times_gpu))} "
                      f"(±{format_time(statistics.stdev(times_gpu) if len(times_gpu) > 1 else 0)}) "
                      f"[{speedup:.2f}x]")

    return results


# ---------------------------------------------------------------------------
#  Benchmark: Boolean Operations
# ---------------------------------------------------------------------------

def benchmark_boolean_operations(polygon_counts, iterations=5, warmup=2, verbose=False):
    """
    Benchmark boolean operations (AND, OR, XOR, NOT) between regions.

    Boolean operations on large polygon sets can benefit from GPU-accelerated
    sub-operations (clipping, merging).
    """
    results = []

    for count in polygon_counts:
        if verbose:
            print(f"  Boolean (AND): {format_count(count)} polygons...", flush=True)

        region_a = generate_region(count, bbox_size=50000)
        region_b = generate_region(count, bbox_size=50000)

        # CPU benchmark
        def run_cpu():
            if hasattr(db, "GPUBackend"):
                db.GPUBackend.set_enabled(False)
            try:
                _ = region_a & region_b
            finally:
                if hasattr(db, "GPUBackend"):
                    db.GPUBackend.set_enabled(True)

        times_cpu = run_benchmark(run_cpu, iterations, warmup)
        results.append(BenchmarkResult("boolean_and", "cpu", count, times_cpu))

        if verbose:
            print(f"    CPU: {format_time(statistics.mean(times_cpu))} "
                  f"(±{format_time(statistics.stdev(times_cpu) if len(times_cpu) > 1 else 0)})")

        # GPU benchmark
        if hasattr(db, "GPUBackend"):
            def run_gpu():
                db.GPUBackend.set_enabled(True)
                _ = region_a & region_b

            times_gpu = run_benchmark(run_gpu, iterations, warmup)
            results.append(BenchmarkResult("boolean_and", "gpu", count, times_gpu))

            if verbose:
                speedup = statistics.mean(times_cpu) / statistics.mean(times_gpu)
                print(f"    GPU: {format_time(statistics.mean(times_gpu))} "
                      f"(±{format_time(statistics.stdev(times_gpu) if len(times_gpu) > 1 else 0)}) "
                      f"[{speedup:.2f}x]")

    return results


# ---------------------------------------------------------------------------
#  Benchmark: DRC Space/Width Checks
# ---------------------------------------------------------------------------

def benchmark_drc_checks(polygon_counts, iterations=5, warmup=2, verbose=False):
    """
    Benchmark DRC space and width checks.

    Design Rule Checks (DRC) for minimum space and width are computationally
    intensive operations that process many polygon interactions.
    """
    results = []

    for count in polygon_counts:
        if verbose:
            print(f"  DRC space check: {format_count(count)} polygons...", flush=True)

        region = generate_region(count, bbox_size=100000)
        min_space = 500  # minimum space in database units

        # CPU benchmark
        def run_cpu():
            if hasattr(db, "GPUBackend"):
                db.GPUBackend.set_enabled(False)
            try:
                _ = region.space_check(min_space)
            finally:
                if hasattr(db, "GPUBackend"):
                    db.GPUBackend.set_enabled(True)

        times_cpu = run_benchmark(run_cpu, iterations, warmup)
        results.append(BenchmarkResult("drc_space_check", "cpu", count, times_cpu))

        if verbose:
            print(f"    CPU: {format_time(statistics.mean(times_cpu))} "
                  f"(±{format_time(statistics.stdev(times_cpu) if len(times_cpu) > 1 else 0)})")

        # GPU benchmark
        if hasattr(db, "GPUBackend"):
            def run_gpu():
                db.GPUBackend.set_enabled(True)
                _ = region.space_check(min_space)

            times_gpu = run_benchmark(run_gpu, iterations, warmup)
            results.append(BenchmarkResult("drc_space_check", "gpu", count, times_gpu))

            if verbose:
                speedup = statistics.mean(times_cpu) / statistics.mean(times_gpu)
                print(f"    GPU: {format_time(statistics.mean(times_gpu))} "
                      f"(±{format_time(statistics.stdev(times_gpu) if len(times_gpu) > 1 else 0)}) "
                      f"[{speedup:.2f}x]")

    return results


# ---------------------------------------------------------------------------
#  Results reporting
# ---------------------------------------------------------------------------

def print_results_table(results):
    """Print results in a formatted table."""
    if not results:
        return

    # Group results by operation
    by_operation = defaultdict(list)
    for r in results:
        by_operation[r.operation].append(r)

    for op, op_results in by_operation.items():
        print(f"\n{'=' * 72}")
        print(f" Operation: {op}")
        print(f"{'=' * 72}")
        print(f"{'Size':>10} {'Backend':>8} {'Mean':>12} {'Median':>12} "
              f"{'StdDev':>12} {'Min':>12} {'Speedup':>10}")
        print(f"{'-' * 10} {'-' * 8} {'-' * 12} {'-' * 12} "
              f"{'-' * 12} {'-' * 12} {'-' * 10}")

        # Group by size for speedup calculation
        by_size = defaultdict(dict)
        for r in op_results:
            by_size[r.size][r.backend] = r

        for size in sorted(by_size.keys()):
            for backend in ["cpu", "gpu"]:
                if backend not in by_size[size]:
                    continue
                r = by_size[size][backend]
                speedup = ""
                if backend == "gpu" and "cpu" in by_size[size]:
                    cpu_mean = by_size[size]["cpu"].mean
                    if r.mean > 0:
                        speedup = f"{cpu_mean / r.mean:.2f}x"

                print(f"{format_count(size):>10} {backend:>8} "
                      f"{format_time(r.mean):>12} {format_time(r.median):>12} "
                      f"{format_time(r.stdev):>12} {format_time(r.min_time):>12} "
                      f"{speedup:>10}")


def save_results_json(results, filepath):
    """Save benchmark results to a JSON file."""
    data = {
        "metadata": {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
            "python_version": sys.version,
            "gpu_info": detect_gpu_info(),
        },
        "results": [r.to_dict() for r in results],
    }
    with open(filepath, "w") as f:
        json.dump(data, f, indent=2)
    print(f"\nResults saved to: {filepath}")


def save_results_csv(results, filepath):
    """Save benchmark results to a CSV file."""
    import csv
    with open(filepath, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "operation", "backend", "size", "mean_seconds",
            "median_seconds", "stdev_seconds", "min_seconds", "max_seconds",
            "iterations"
        ])
        for r in results:
            writer.writerow([
                r.operation, r.backend, r.size, r.mean,
                r.median, r.stdev, r.min_time, r.max_time,
                len(r.times)
            ])
    print(f"Results saved to: {filepath}")


def plot_results(results, output_file=None):
    """Plot benchmark results using matplotlib."""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Warning: matplotlib not available, skipping plots.", file=sys.stderr)
        return

    # Group results by operation
    by_operation = defaultdict(list)
    for r in results:
        by_operation[r.operation].append(r)

    num_ops = len(by_operation)
    if num_ops == 0:
        return

    fig, axes = plt.subplots(1, num_ops, figsize=(6 * num_ops, 5), squeeze=False)

    for idx, (op, op_results) in enumerate(sorted(by_operation.items())):
        ax = axes[0][idx]

        # Separate CPU and GPU results
        cpu_results = sorted([r for r in op_results if r.backend == "cpu"],
                             key=lambda r: r.size)
        gpu_results = sorted([r for r in op_results if r.backend == "gpu"],
                             key=lambda r: r.size)

        if cpu_results:
            sizes = [r.size for r in cpu_results]
            means = [r.mean * 1000 for r in cpu_results]  # Convert to ms
            stdevs = [r.stdev * 1000 for r in cpu_results]
            ax.errorbar(sizes, means, yerr=stdevs, marker='o', label='CPU',
                        capsize=3, linewidth=2)

        if gpu_results:
            sizes = [r.size for r in gpu_results]
            means = [r.mean * 1000 for r in gpu_results]
            stdevs = [r.stdev * 1000 for r in gpu_results]
            ax.errorbar(sizes, means, yerr=stdevs, marker='s', label='GPU',
                        capsize=3, linewidth=2)

        ax.set_xlabel("Problem Size (polygon/point count)")
        ax.set_ylabel("Time (ms)")
        ax.set_title(op.replace("_", " ").title())
        ax.legend()
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.grid(True, alpha=0.3)

    plt.tight_layout()

    if output_file:
        plt.savefig(output_file, dpi=150, bbox_inches="tight")
        print(f"Plot saved to: {output_file}")
    else:
        plt.show()


# ---------------------------------------------------------------------------
#  Main entry point
# ---------------------------------------------------------------------------

DEFAULT_SIZES = [100, 500, 1000, 5000, 10000, 50000, 100000]

OPERATION_MAP = {
    "sizing": benchmark_sizing,
    "pip": benchmark_point_in_polygon,
    "bbox": benchmark_bounding_boxes,
    "boolean": benchmark_boolean_operations,
    "drc": benchmark_drc_checks,
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="KLayout GPU vs CPU Benchmarking Suite",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--operations", nargs="+", default=["all"],
        choices=list(OPERATION_MAP.keys()) + ["all"],
        help="Operations to benchmark (default: all)",
    )
    parser.add_argument(
        "--sizes", nargs="+", type=int, default=None,
        help=f"Problem sizes to test (default: {DEFAULT_SIZES})",
    )
    parser.add_argument(
        "--iterations", type=int, default=5,
        help="Number of timed iterations per measurement (default: 5)",
    )
    parser.add_argument(
        "--warmup", type=int, default=2,
        help="Number of warmup iterations (default: 2)",
    )
    parser.add_argument(
        "--output", type=str, default=None,
        help="Output results to JSON file",
    )
    parser.add_argument(
        "--csv", type=str, default=None,
        help="Output results to CSV file",
    )
    parser.add_argument(
        "--plot", type=str, default=None,
        help="Save plot to file (PNG/PDF/SVG)",
    )
    parser.add_argument(
        "--no-plot", action="store_true",
        help="Disable interactive plotting",
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true",
        help="Print detailed progress information",
    )
    parser.add_argument(
        "--seed", type=int, default=42,
        help="Random seed for reproducibility (default: 42)",
    )
    return parser.parse_args()


def main():
    args = parse_args()

    # Set random seed for reproducibility
    random.seed(args.seed)

    sizes = args.sizes if args.sizes else DEFAULT_SIZES

    # Determine which operations to run
    if "all" in args.operations:
        operations = list(OPERATION_MAP.keys())
    else:
        operations = args.operations

    # Print header
    print("=" * 72)
    print(" KLayout GPU vs CPU Benchmark Suite")
    print("=" * 72)
    print(f" KLayout version: {db.Application if hasattr(db, 'Application') else 'N/A'}")
    print(f" Python version:  {sys.version.split()[0]}")

    # Detect GPU
    gpu_info = detect_gpu_info()
    if gpu_info["available"]:
        print(f" GPU backend:     {gpu_info['backend']}")
        print(f" GPU device:      {gpu_info['device_name']}")
    else:
        print(" GPU backend:     Not available (CPU-only benchmarks)")
        if not hasattr(db, "GPUBackend"):
            print(" Note: GPUBackend not exposed to Python API.")
            print("       Running CPU-only benchmarks for baseline measurement.")

    print(f" Operations:      {', '.join(operations)}")
    print(f" Problem sizes:   {sizes}")
    print(f" Iterations:      {args.iterations} (warmup: {args.warmup})")
    print(f" Random seed:     {args.seed}")
    print("=" * 72)
    print()

    # Run benchmarks
    all_results = []

    for op_name in operations:
        print(f"Running benchmark: {op_name}")
        bench_func = OPERATION_MAP[op_name]
        try:
            results = bench_func(
                sizes,
                iterations=args.iterations,
                warmup=args.warmup,
                verbose=args.verbose,
            )
            all_results.extend(results)
        except Exception as e:
            print(f"  ERROR: {e}", file=sys.stderr)
            if args.verbose:
                import traceback
                traceback.print_exc()
        print()

    # Print results table
    print_results_table(all_results)

    # Save results
    if args.output:
        save_results_json(all_results, args.output)

    if args.csv:
        save_results_csv(all_results, args.csv)

    # Plot results
    if not args.no_plot:
        if args.plot:
            plot_results(all_results, output_file=args.plot)
        elif sys.stdout.isatty():
            plot_results(all_results)

    print(f"\nBenchmark complete. {len(all_results)} measurements recorded.")


if __name__ == "__main__":
    main()
