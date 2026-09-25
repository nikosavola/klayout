# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib>=3.8,<4", "pandas>=2.2,<3"]
# ///

# Speedup of the PR 2365/2367 execution-policy sorts versus CPU count. Reads
# Google Benchmark JSON files named "<build>-cpus<N>-pass<P>.json" from the
# sweep in scripts/benchmark_pr_sorts.sh.

import argparse
import json
import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd

TIME_TO_MS = {"ns": 1e-6, "us": 1e-3, "ms": 1.0, "s": 1e3}
FILE_RE = re.compile(r"(?P<build>.+)-cpus(?P<cpus>\d+)-pass(?P<pass>\d+)\.json$")
CASE_RE = re.compile(r"(?P<case>[\w]+)/(?P<size>\d+)(?:/real_time)?$")

CASES = ("region_merge_properties", "shapes_erase", "plc_decomposition")
CASE_TITLES = {
    "region_merge_properties": "Region merge, property-aware (#2365)",
    "shapes_erase": "Shape erasure (#2367)",
    "plc_decomposition": "PLC decomposition (#2367)",
}
CASE_UNITS = {"region_merge_properties": "polygons", "shapes_erase": "polygons", "plc_decomposition": "contours"}

BUILD_COLORS = {"before": "#eb6834", "after": "#2a78d6"}
INK = "#52514e"

matplotlib.rcParams.update({"font.size": 11, "axes.titlesize": 12})


def load_results(results_dir: Path) -> pd.DataFrame:
    rows = []
    for path in sorted(results_dir.glob("*.json")):
        if not (file_match := FILE_RE.fullmatch(path.name)):
            continue
        data = json.loads(path.read_text())
        for entry in data["benchmarks"]:
            if entry.get("aggregate_name") or entry.get("run_type") == "aggregate":
                continue
            if entry.get("error_occurred"):
                raise ValueError(f"{path}: {entry['name']}: {entry.get('error_message', 'benchmark failed')}")
            if not (case_match := CASE_RE.fullmatch(entry["name"])) or case_match["case"] not in CASES:
                continue
            rows.append(
                {
                    "build": file_match["build"],
                    "cpus": int(file_match["cpus"]),
                    "pass": int(file_match["pass"]),
                    "case": case_match["case"],
                    "size": int(case_match["size"]),
                    "milliseconds": entry["real_time"] * TIME_TO_MS[entry["time_unit"]],
                }
            )
    if not rows:
        raise ValueError(f"no <build>-cpus<N>-pass<P>.json results in {results_dir}")
    return pd.DataFrame(rows)


def pass_medians(frame: pd.DataFrame) -> pd.DataFrame:
    return frame.groupby(["build", "cpus", "pass", "case", "size"], as_index=False)["milliseconds"].median()


def paired_speedups(medians: pd.DataFrame, baseline: str, candidate: str) -> pd.DataFrame:
    before = medians[medians["build"] == baseline]
    after = medians[medians["build"] == candidate]
    speedups = before.merge(after, on=["cpus", "pass", "case", "size"], suffixes=("_baseline", "_candidate"))
    speedups["speedup"] = speedups["milliseconds_baseline"] / speedups["milliseconds_candidate"]
    return speedups.groupby(["cpus", "case", "size"], as_index=False)["speedup"].agg(["median", "min", "max"])


def style_axes(ax: plt.Axes, cpus: list[int], xlabel: str) -> None:
    ax.set_xlabel(xlabel)
    ax.set_xticks(cpus)
    ax.grid(alpha=0.25)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for artist in (ax.xaxis.label, ax.yaxis.label):
        artist.set_color(INK)
    ax.tick_params(colors=INK)
    for spine in ax.spines.values():
        spine.set_color(INK)


def save(fig: plt.Figure, output: Path) -> None:
    fig.savefig(output)
    fig.savefig(output.with_suffix(".png"), dpi=150)
    plt.close(fig)


def plot_speedup(speedups: pd.DataFrame, baseline: str, output: Path) -> None:
    cases = [case for case in CASES if case in set(speedups["case"])]
    fig, axes = plt.subplots(1, len(cases), figsize=(4.6 * len(cases), 4.0), squeeze=False, sharex=True)
    for ax, case in zip(axes[0], cases, strict=True):
        unit = CASE_UNITS[case]
        case_speedups = speedups[speedups["case"] == case].sort_values(["size", "cpus"])
        size_colors = dict(zip(sorted(case_speedups["size"].unique()), ["#2a78d6", "#eb6834"], strict=False))
        for size, group in case_speedups.groupby("size"):
            color = size_colors.get(size, "#008300")
            ax.plot(group["cpus"], group["median"], marker="o", markersize=5, linewidth=2, color=color, label=f"{size} {unit}")
            ax.fill_between(group["cpus"], group["min"], group["max"], color=color, alpha=0.2, linewidth=0)
        ax.axhline(1.0, color="#9a9994", linewidth=1, zorder=0)
        ax.set_ylim(bottom=0)
        ax.set_title(CASE_TITLES[case], color=INK)
        ax.set_ylabel(f"Speedup versus {baseline}")
        ax.legend(frameon=False, labelcolor=INK)
        style_axes(ax, sorted(case_speedups["cpus"].unique()), "CPUs available")
    fig.suptitle("Parallel sort speedup versus CPU count", color=INK)
    fig.tight_layout()
    save(fig, output)


def plot_runtime(medians: pd.DataFrame, output: Path) -> None:
    cases = [case for case in CASES if case in set(medians["case"])]
    fig, axes = plt.subplots(1, len(cases), figsize=(4.6 * len(cases), 4.2), squeeze=False)
    for ax, case in zip(axes[0], cases, strict=True):
        unit = CASE_UNITS[case]
        case_medians = medians[medians["case"] == case]
        largest = case_medians["size"].max()
        for (build, size), group in sorted(case_medians.groupby(["build", "size"])):
            group = group.sort_values("cpus")
            ax.plot(
                group["cpus"],
                group["milliseconds"],
                marker="o",
                markersize=5,
                linewidth=2,
                linestyle="-" if size == largest else "--",
                color=BUILD_COLORS.get(build, "#008300"),
                label=f"{build}, {size} {unit}",
            )
        ax.set_yscale("log")
        ax.set_title(CASE_TITLES[case], color=INK)
        ax.set_ylabel("Median wall time (ms)")
        ax.legend(frameon=False, labelcolor=INK)
        style_axes(ax, sorted(case_medians["cpus"].unique()), "CPUs available")
    fig.suptitle("Runtime versus CPU count", color=INK)
    fig.tight_layout()
    save(fig, output)


def print_table(medians: pd.DataFrame, baseline: str, candidate: str) -> None:
    median_runs = medians.groupby(["case", "size", "cpus", "build"], as_index=False)["milliseconds"].median()
    table = median_runs.pivot(index=["case", "size", "cpus"], columns="build", values="milliseconds")
    table["speedup"] = table[baseline] / table[candidate]
    print(f"| case | size | CPUs | {baseline} (ms) | {candidate} (ms) | speedup |")
    print("| --- | --- | --- | --- | --- | --- |")
    for (case, size, cpus), row in table.iterrows():
        print(f"| {case} | {int(size)} | {int(cpus)} | {row[baseline]:.3f} | {row[candidate]:.3f} | {row['speedup']:.2f}x |")


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot PR 2365/2367 sort speedup versus CPU count")
    parser.add_argument("results_dir", type=Path)
    parser.add_argument("--baseline", default="before")
    parser.add_argument("--candidate", default="after")
    parser.add_argument("--output-dir", type=Path, default=Path("pr-sort-plots"))
    args = parser.parse_args()

    medians = pass_medians(load_results(args.results_dir))
    for build in (args.baseline, args.candidate):
        if build not in set(medians["build"]):
            parser.error(f"no results for build '{build}' in {args.results_dir}")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    plot_speedup(paired_speedups(medians, args.baseline, args.candidate), args.baseline, args.output_dir / "speedup-vs-cpus.svg")
    plot_runtime(medians, args.output_dir / "runtime-vs-cpus.svg")
    print_table(medians, args.baseline, args.candidate)


if __name__ == "__main__":
    main()