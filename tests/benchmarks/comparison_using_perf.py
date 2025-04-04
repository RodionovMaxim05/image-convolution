import os
import numpy as np
import matplotlib.pyplot as plt
import re
import subprocess
from utils import analyze_execution_data

# Configuration constants
PROGRAM_PATH = "./build/src/image-convolution"
IMAGE_NAME = "cat.bmp"
IMAGE_PATH = "images/" + IMAGE_NAME
OUTPUT_DIR = "tests/plots"
NUM_RUNS = 40
THREAD_NUM = 4

MODES = ["seq", "pixel", "row", "column", "block"]

os.makedirs(OUTPUT_DIR, exist_ok=True)


def run_perf_stat(mode: str):
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
    results = {
        "cache-references": {"means": [], "conf_inter": []},
        "cache-misses": {"means": [], "conf_inter": []},
        "L1-dcache-load_misses": {"means": [], "conf_inter": []},
    }

    # Run perf for all modes
    for mode in MODES:
        print(f"Running perf measurements: {mode} mode")
        c_ref, c_mis, l1_mis = run_perf_stat(mode)

        for metric, data in zip(
            ["cache-references", "cache-misses", "L1-dcache-load_misses"],
            [c_ref, c_mis, l1_mis],
        ):
            mean_val, conf_inter = analyze_execution_data(data)
            results[metric]["means"].append(mean_val)
            results[metric]["conf_inter"].append(conf_inter)

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

        individual_chart_path = os.path.join(OUTPUT_DIR, f"histogram_perf_{option}.png")
        plt.savefig(individual_chart_path)
        plt.clf()

    print(f"Measurements and charts saved to '{OUTPUT_DIR}' directory.")


if __name__ == "__main__":
    main()
