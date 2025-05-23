import os
import numpy as np
import matplotlib.pyplot as plt
import re
import subprocess
import sys
from utils import analyze_execution_data, get_image_resolution

# Configuration constants
PROGRAM_PATH = "./build/src/image-convolution"
IMAGE_NAME = sys.argv[1]
IMAGE_PATH = "images/" + IMAGE_NAME
IMAGE_RESOLUTION = get_image_resolution(IMAGE_PATH)
OUTPUT_DIR = f"tests/plots/{IMAGE_RESOLUTION[0]}x{IMAGE_RESOLUTION[1]}"
NUM_RUNS = 40
THREAD_NUM = 4

MODES = ["seq", "pixel", "row", "column", "block"]

os.makedirs(OUTPUT_DIR, exist_ok=True)


def run_perf_stat(mode: str):
    """
    Runs the `perf stat` command to measure performance metrics for a specific execution mode.

    Args:
        mode (str): Execution mode.

    Returns:
        A tuple (c_ref, c_mis, l1_mis) containing NumPy arrays of:
            c_ref: Cache references.
            c_mis: Cache misses.
            l1_mis: L1 data cache load misses.
    """
    c_ref, c_mis, l1_mis = [], [], []
    if mode == "seq":
        command = f"perf stat -e cache-references,cache-misses,L1-dcache-load-misses {PROGRAM_PATH} {IMAGE_PATH} id --mode={mode}"
    else:
        command = f"perf stat -e cache-references,cache-misses,L1-dcache-load-misses {PROGRAM_PATH} {IMAGE_PATH} id --mode={mode} --thread={THREAD_NUM}"

    for _ in range(NUM_RUNS):
        process = subprocess.Popen(
            command,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        stdout, stderr = process.communicate()
        output = stdout + stderr

        # Parse the output of perf stat
        for line in output.splitlines():
            line = line.replace(" ", "").replace(",", "")

            if "cache-references" in line:
                match = re.search(r"(\d+)\s+cache-references", line)
                c_ref.append(int(match.group(1)))
            elif "cache-misses" in line:
                match = re.search(r"(\d+)\s+cache-misses", line)
                c_mis.append(int(match.group(1)))
            elif "L1-dcache-load-misses" in line:
                match = re.search(r"(\d+)\s+L1-dcache-load-misses", line)
                l1_mis.append(int(match.group(1)))

    return np.array(c_ref), np.array(c_mis), np.array(l1_mis)


def main() -> None:
    """
    The core function organizes the performance measurement process,
    including running tests, analyzing results, storing results, and creating graphs.
    """
    data_file = open(f"{OUTPUT_DIR}/perf_results.txt", "w+")

    results = {
        "cache-references": {"means": [], "conf_inter": []},
        "cache-misses": {"means": [], "conf_inter": []},
        "L1-dcache-load_misses": {"means": [], "conf_inter": []},
    }

    # Run perf for all modes
    for i, mode in enumerate(MODES):
        data_file.write(f"MODE: {mode}\n")

        print(f"Running perf measurements: {mode} mode")
        c_ref, c_mis, l1_mis = run_perf_stat(mode)

        for metric, data in zip(
            ["cache-references", "cache-misses", "L1-dcache-load_misses"],
            [c_ref, c_mis, l1_mis],
        ):
            mean_val, conf_inter = analyze_execution_data(data, filter_data=True)
            rounded_mean, rounded_error = int(mean_val), int(conf_inter)
            results[metric]["means"].append(rounded_mean)
            results[metric]["conf_inter"].append(rounded_error)

        # Save perf results to a text file
        data_file.write(
            f"\tCache-references: {results["cache-references"]["means"][i]} ± {results["cache-references"]["conf_inter"][i]}\n"
        )
        data_file.write(
            f"\tCache-misses: {results["cache-misses"]["means"][i]} ± {results["cache-misses"]["conf_inter"][i]}\n"
        )
        data_file.write(
            f"\tL1-dcache-load_misses: {results["L1-dcache-load_misses"]["means"][i]} ± {results["L1-dcache-load_misses"]["conf_inter"][i]}\n\n"
        )

    data_file.close()

    for option in results:
        x_pos = np.arange(len(MODES))

        plt.bar(
            x=x_pos,
            height=results[option]["means"],
            yerr=results[option]["conf_inter"],
            align="center",
            alpha=0.7,
            capsize=10,
            color=(["#42aaff", "#efa94a", "#47a76a", "#db5856", "#9966cc"]),
        )
        plt.xticks(x_pos, MODES)
        plt.ylabel("Count")
        plt.title(f"Perf Option '{option}'")

        chart_path = os.path.join(OUTPUT_DIR, f"histogram_perf_{option}.png")
        plt.savefig(chart_path)
        plt.clf()

    print(f"Measurements and charts saved to '{OUTPUT_DIR}' directory.")


if __name__ == "__main__":
    main()
