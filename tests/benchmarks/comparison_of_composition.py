import os
import numpy as np
import re
import sys
import matplotlib.pyplot as plt
from utils import (
    analyze_execution_data,
    print_warning,
    get_image_resolution,
    round_results,
)


# Configuration constants
PROGRAM_PATH = "./build/src/image-convolution"
IMAGE_NAME = sys.argv[1]
IMAGE_PATH = "images/" + IMAGE_NAME
IMAGE_RESOLUTION = get_image_resolution(IMAGE_PATH)
OUTPUT_DIR = f"tests/plots/{IMAGE_RESOLUTION[0]}x{IMAGE_RESOLUTION[1]}"
NUM_RUNS = 40

# Define pairs of filters to test
FILTER_PAIRS = [
    ("bl", "gbl"),  # Blur + Gaussian Blur
    ("fbl", "mbl"),  # Fast Blur + Motion Blur
]

os.makedirs(OUTPUT_DIR, exist_ok=True)


def run_program(filter1: str, filter2: str, is_seq: bool):
    """
    Runs the convolution program and parses the execution time of
    applying two filters in sequence or a composition of these filters.

    Args:
        filter1 (str): Name of the first filter.
        filter2 (str): Name of the second filter.
        is_seq (bool): Mode of application (`True` for sequential, `False` for composition).

    Returns:
        A NumPy array containing execution times for all runs.
    """
    execution_times = []

    for _ in range(NUM_RUNS):
        if is_seq:
            # Run the first filter
            command1 = f"{PROGRAM_PATH} {IMAGE_PATH} {filter1} --mode=seq"
            output = os.popen(command1).read().strip()

            match_time1 = re.search(r"The convolution took (\d+\.\d+)", output)
            if not match_time1:
                print_warning(f"Failed to parse time from output: {output}")
            time = float(match_time1.group(1))

            # Get the intermediate image path
            match_path = re.search(r"The final image is located at '(.+?)'", output)
            if not match_path:
                print_warning(f"Failed to parse file path from output: {output}")

            file_path = match_path.group(1)

            # Run the second filter
            command2 = f"{PROGRAM_PATH} {file_path} {filter2} --mode=seq"
            output = os.popen(command2).read().strip()

            match_time2 = re.search(r"The convolution took (\d+\.\d+)", output)
            if not match_time2:
                print_warning(f"Failed to parse time from output: {output}")
            time += float(match_time2.group(1))

        else:
            # Run the composed filter
            command = f"{PROGRAM_PATH} {IMAGE_PATH} {filter1}+{filter2} --mode=seq"
            output = os.popen(command).read().strip()

            match_time = re.search(r"The convolution took (\d+\.\d+)", output)
            if not match_time:
                print_warning(f"Failed to parse time from output: {output}")
            time = float(match_time.group(1))

        execution_times.append(time)

    return np.array(execution_times)


def main() -> None:
    """
    The main function organizes the process of comparative analysis of the execution time of
    the sequential application of two filters using their composition, including conducting tests,
    analyzing results, creating graphs and saving results.
    """
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    data_file = open(f"{OUTPUT_DIR}/composition_results.txt", "w+")

    results = [{"means": [], "conf_inter": []} for _ in FILTER_PAIRS]

    for num, (filter1, filter2) in enumerate(FILTER_PAIRS):
        print(
            f"Comparing sequential vs composed for filters: {filter1} and {filter2}"
        )

        # Parse sequential application time
        seq_time = run_program(filter1, filter2, True)

        data_file.write(f"Result for {filter1} + {filter2}\n")

        mean_time, confidence_interval = analyze_execution_data(
            seq_time, filter_data=True
        )
        results[num]["means"].append(mean_time)
        results[num]["conf_inter"].append(confidence_interval)

        rounded_mean, rounded_error = round_results(mean_time, confidence_interval)
        data_file.write(f"\tSequential: {rounded_mean} ± {rounded_error} s\n")

        # Parse composed application time
        composed_time = run_program(filter1, filter2, False)

        mean_time, confidence_interval = analyze_execution_data(composed_time)
        results[num]["means"].append(mean_time)
        results[num]["conf_inter"].append(confidence_interval)

        rounded_mean, rounded_error = round_results(mean_time, confidence_interval)
        data_file.write(f"\tComposed: {rounded_mean} ± {rounded_error} s\n")

    data_file.close()

    # Create a bar chart for the comparison
    x_labels = [f"{filter1} + {filter2}" for filter1, filter2 in FILTER_PAIRS]

    x_pos = np.arange(len(x_labels))
    bar_width = 0.35

    colors = ["#3E5F8A", "#633A34"]

    for i in range(0, 2):
        plt.bar(
            x=x_pos + (i - 0.5) * bar_width,
            height=[result["means"][i] for result in results],
            yerr=[result["conf_inter"][i] for result in results],
            capsize=5,
            width=bar_width,
            label="Sequential" if i % 2 == 0 else "Composed",
            alpha=0.7,
            color=colors[i],
            error_kw={"elinewidth": 1.5},
        )

    plt.xticks(x_pos, x_labels)
    plt.ylabel("Execution Time (s)")
    plt.title("Comparison: Sequential vs Composed Filters")
    plt.legend()

    comparison_chart_path = os.path.join(
        OUTPUT_DIR,
        f"comparison_sequential_vs_composed.png",
    )
    plt.tight_layout()
    plt.savefig(comparison_chart_path)

    print(f"Measurements and charts saved to '{OUTPUT_DIR}' directory.")


if __name__ == "__main__":
    main()
