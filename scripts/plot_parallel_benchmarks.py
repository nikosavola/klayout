# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib>=3.8,<4", "pandas>=2.2,<3"]
# ///

import argparse
import json
import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import pandas as pd


TIME_TO_MS = {"ns": 1e-6, "us": 1e-3, "ms": 1.0, "s": 1e3}
SCALING_CASE = re.compile(r"^(hierarchical_and|compound_bool_or|compound_interact|hierarchical_connectivity)/(\d+)/(\d+)/real_time$")


def format_case(name: str) -> str:
    if match := SCALING_CASE.fullmatch(name):
        workload, threads, size = match.groups()
        unit = "cells" if workload == "hierarchical_connectivity" else "boxes/cell"
        return f"{workload.replace('_', ' ')} | t={threads}, {size} {unit}"
    workload, size, *_ = name.split("/")
    unit = {"edge_merge": "boxes", "netlist_compare": "devices", "polygon_rasterize": "vertices"}.get(workload, "items")
    return f"{workload.replace('_', ' ')} | {size} {unit}"


def load_results(spec: str) -> pd.DataFrame:
    label, path_text = spec.split("=", 1)
    path = Path(path_text)
    data = json.loads(path.read_text())
    rows = []
    for entry in data["benchmarks"]:
        if entry.get("aggregate_name") or entry.get("run_type") == "aggregate":
            continue
        if entry.get("error_occurred"):
            raise ValueError(f"{path}: {entry['name']}: {entry.get('error_message', 'benchmark failed')}")
        rows.append(
            {
                "build": label,
                "name": re.sub(r"/repeats:\d+", "", entry["name"]),
                "milliseconds": entry["real_time"] * TIME_TO_MS[entry["time_unit"]],
            }
        )
    if not rows:
        raise ValueError(f"{path}: no benchmark measurements")
    return pd.DataFrame(rows).groupby(["build", "name"], as_index=False)["milliseconds"].median()


def plot_runtime(results: pd.DataFrame, output: Path) -> None:
    table = results.pivot(index="name", columns="build", values="milliseconds").sort_index()
    table.index = [format_case(name) for name in table.index]
    fig, ax = plt.subplots(figsize=(11, max(6, len(table) * 0.35)))
    table.plot.barh(ax=ax, width=0.8)
    ax.set_xscale("log")
    ax.set_xlabel("Median wall time (ms, log scale; lower is better)")
    ax.set_ylabel("")
    ax.set_title("Parallel branch workloads")
    ax.grid(axis="x", alpha=0.25)
    ax.legend(title="Build")
    fig.tight_layout()
    fig.savefig(output)
    plt.close(fig)


def plot_build_speedup(results: pd.DataFrame, baseline: str, output: Path) -> None:
    table = results.pivot(index="name", columns="build", values="milliseconds").sort_index()
    if table.isna().any().any():
        raise ValueError("Builds must contain the same benchmark cases for a speedup comparison")
    speedup = table.drop(columns=baseline).rdiv(table[baseline], axis=0)
    speedup.index = [format_case(name) for name in speedup.index]
    fig, ax = plt.subplots(figsize=(11, max(6, len(table) * 0.35)))
    speedup.plot.barh(ax=ax, width=0.8)
    ax.axvline(1, color="black", linewidth=1)
    ax.set_xlabel(f"Speedup versus {baseline} (higher is better)")
    ax.set_ylabel("")
    ax.set_title("Change between builds")
    ax.grid(axis="x", alpha=0.25)
    ax.legend(title="Build")
    fig.tight_layout()
    fig.savefig(output)
    plt.close(fig)


def plot_thread_scaling(results: pd.DataFrame, output: Path) -> None:
    scaling = results.copy()
    parsed = scaling["name"].str.extract(SCALING_CASE)
    scaling[["workload", "threads", "size"]] = parsed
    scaling = scaling.dropna(subset=["workload"]).copy()
    scaling[["threads", "size"]] = scaling[["threads", "size"]].astype(int)
    if scaling.empty:
        return

    workloads = scaling["workload"].unique()
    fig, axes = plt.subplots(1, len(workloads), figsize=(6 * len(workloads), 5), squeeze=False)
    for ax, workload in zip(axes[0], workloads):
        cases = scaling[scaling["workload"] == workload]
        for (build, size), group in cases.groupby(["build", "size"]):
            group = group.sort_values("threads")
            baseline = group.iloc[0]["milliseconds"]
            unit = "cells" if workload == "hierarchical_connectivity" else "boxes/cell"
            ax.plot(group["threads"], baseline / group["milliseconds"], marker="o", label=f"{build}, {size} {unit}")
        ax.axhline(1, color="black", linewidth=1)
        ax.set_title(workload.replace("_", " ").title())
        ax.set_xlabel("OpenMP threads" if workload == "hierarchical_connectivity" else "Configured threads (0 = serial)")
        ax.set_ylabel("Speedup versus lowest thread setting")
        ax.set_xticks(sorted(cases["threads"].unique()))
        ax.grid(alpha=0.25)
        ax.legend()
    fig.tight_layout()
    fig.savefig(output)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot Google Benchmark JSON from parallel KLayout builds")
    parser.add_argument("results", nargs="+", metavar="LABEL=FILE", help="JSON results; first build is the speedup baseline")
    parser.add_argument("--output-dir", type=Path, default=Path("benchmark-plots"))
    args = parser.parse_args()

    try:
        labels = [spec.split("=", 1)[0] for spec in args.results]
        if len(labels) != len(set(labels)) or any(not label for label in labels):
            raise ValueError("Each result needs a unique nonempty label")
        results = pd.concat([load_results(spec) for spec in args.results], ignore_index=True)
    except (KeyError, ValueError) as error:
        parser.error(str(error))

    args.output_dir.mkdir(parents=True, exist_ok=True)
    plot_runtime(results, args.output_dir / "runtime.svg")
    plot_thread_scaling(results, args.output_dir / "thread-scaling.svg")
    if len(labels) > 1:
        try:
            plot_build_speedup(results, labels[0], args.output_dir / "build-speedup.svg")
        except ValueError as error:
            parser.error(str(error))


if __name__ == "__main__":
    main()
