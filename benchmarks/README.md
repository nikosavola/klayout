# KLayout GPU vs CPU Benchmarking Suite

This directory contains a Python-based benchmarking suite for comparing KLayout's
GPU-accelerated operations against their CPU counterparts.

## Overview

The benchmarking suite measures performance for the following operations:

| Operation | Description | GPU Benefit |
|-----------|-------------|-------------|
| **Polygon Sizing** | Offset/inflate/deflate polygons | High (embarrassingly parallel) |
| **Point-in-Polygon** | Batch point containment tests | Very High (independent per point) |
| **Bounding Boxes** | Batch bounding box computation | Moderate (very cheap per polygon) |
| **Boolean Operations** | Region AND/OR/XOR/NOT | Moderate (sub-operations parallelizable) |
| **DRC Checks** | Space/width design rule checks | High (many polygon interactions) |

## Requirements

- Python 3.6+
- `klayout` Python module (`pip install klayout`)
- Optional: `matplotlib` (for plotting results)

## Quick Start

```bash
# Run all benchmarks with default settings
python gpu_benchmark.py

# Run specific operations with verbose output
python gpu_benchmark.py --operations sizing pip --verbose

# Run with custom problem sizes
python gpu_benchmark.py --sizes 1000 10000 100000 500000

# Save results to files
python gpu_benchmark.py --output results.json --csv results.csv

# Generate a plot
python gpu_benchmark.py --plot benchmark_results.png
```

## Command-Line Options

```
--operations OP [OP ...]   Operations to benchmark:
                           sizing, pip, bbox, boolean, drc, all (default: all)
--sizes N [N ...]          Problem sizes (polygon/point counts) to test
--iterations N             Number of timed iterations per measurement (default: 5)
--warmup N                 Number of warmup iterations (default: 2)
--output FILE              Save results to JSON file
--csv FILE                 Save results to CSV file
--plot FILE                Save plot to image file (PNG/PDF/SVG)
--no-plot                  Disable interactive matplotlib plotting
--verbose, -v              Print detailed progress information
--seed N                   Random seed for reproducibility (default: 42)
```

## Understanding the Results

The benchmark outputs a table showing:

- **Size**: Number of polygons or points in the test
- **Backend**: `cpu` or `gpu`
- **Mean**: Average execution time across iterations
- **Median**: Median execution time
- **StdDev**: Standard deviation (lower = more consistent)
- **Min**: Fastest observed time
- **Speedup**: GPU speedup factor relative to CPU (shown for GPU rows)

### Interpreting Speedup

- **Speedup > 1x**: GPU is faster than CPU
- **Speedup < 1x**: CPU is faster (typically for small problem sizes due to GPU dispatch overhead)
- The crossover point where GPU becomes beneficial depends on the operation and hardware

## GPU Dispatch Thresholds

KLayout uses automatic thresholds to decide when to dispatch work to the GPU:

- **Polygon sizing**: ~10,000 polygons minimum
- **Point-in-polygon**: ~50,000 points minimum
- **Bounding boxes**: ~100,000 polygons minimum (very cheap on CPU)

Below these thresholds, the CPU path is used regardless of GPU availability because
data transfer overhead would negate any GPU computation speedup.

## JSON Output Format

```json
{
  "metadata": {
    "timestamp": "2026-01-15T10:30:00+0000",
    "python_version": "3.11.0",
    "gpu_info": {
      "available": true,
      "backend": "CUDA",
      "device_name": "NVIDIA RTX 4090",
      "total_memory": 25769803776,
      "compute_capability": "8.9"
    }
  },
  "results": [
    {
      "operation": "sizing",
      "backend": "gpu",
      "size": 100000,
      "mean_seconds": 0.045,
      "median_seconds": 0.044,
      "stdev_seconds": 0.002,
      "min_seconds": 0.043,
      "max_seconds": 0.048,
      "iterations": 5
    }
  ]
}
```

## Notes

- When no GPU is available, only CPU benchmarks are run. This is useful for
  establishing baseline performance numbers.
- The random seed ensures reproducible polygon generation across runs.
- Results may vary between runs due to system load, thermal throttling, etc.
  Use multiple iterations and look at median values for the most stable comparison.
- For the most accurate results, close other applications and ensure the system
  is not under heavy load during benchmarking.
