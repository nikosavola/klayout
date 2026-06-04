# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "matplotlib",
#     "pandas",
#     "tabulate"
# ]
# ///
import json
import matplotlib.pyplot as plt
import pandas as pd
import sys
import subprocess

def run_benchmarks():
    commands = []
    threads = [1, 2, 4, 6, 8]
    for t in threads:
        # taskset -c 0-{t-1} limits execution to t cores
        cmd = f"taskset -c 0-{t-1} env LD_LIBRARY_PATH=./bin-release ./bin-release/klayout -b -r test_bool.rb"
        commands.append((t, cmd))
        
    hyperfine_cmd = ["hyperfine", "--export-json", "results.json", "--min-runs", "3"]
    for t, cmd in commands:
        hyperfine_cmd.extend(["-n", f"{t} Threads", cmd])
        
    print("Running hyperfine...")
    subprocess.run(hyperfine_cmd, check=True)
    
    with open("results.json") as f:
        data = json.load(f)
        
    results = []
    for i, t in enumerate(threads):
        res = data["results"][i]
        results.append({
            "Threads": t,
            "Mean Time (s)": res["mean"],
            "Std Dev (s)": res["stddev"]
        })
        
    df = pd.DataFrame(results)
    print(df.to_markdown(index=False))
    
    plt.figure(figsize=(8, 5))
    plt.errorbar(df["Threads"], df["Mean Time (s)"], yerr=df["Std Dev (s)"], marker='o', capsize=5)
    plt.xlabel("Number of Threads")
    plt.ylabel("Mean Execution Time (s)")
    plt.title("Parallel Edge Processor Benchmark")
    plt.grid(True)
    plt.xticks(threads)
    plt.savefig("benchmark_plot.png")
    print("Plot saved as benchmark_plot.png")
    
    # Save the markdown description to a file
    with open("PR_BENCHMARK.md", "w") as f:
        f.write("# Parallel Sorting Benchmark Results\n\n")
        f.write("This PR introduces parallel sorting using C++17 execution policies. ")
        f.write("Here are the benchmark results on boolean operations with edge processors.\n\n")
        f.write("### Benchmark Environment\n")
        f.write("- **Tool**: `hyperfine` (min 3 runs per thread count)\n")
        f.write("- **Command**: `taskset -c <cores> env LD_LIBRARY_PATH=./bin-release ./bin-release/klayout -b -r test_bool.rb`\n")
        f.write("- **Test Case**: Randomly distributed polygon intersections (approx 4M output polygons)\n\n")
        f.write("### Results\n\n")
        f.write(df.to_markdown(index=False))
        f.write("\n\n![Benchmark Plot](benchmark_plot.png)\n")

if __name__ == "__main__":
    run_benchmarks()