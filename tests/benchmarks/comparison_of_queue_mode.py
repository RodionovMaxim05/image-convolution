import os
import numpy as np
import matplotlib.pyplot as plt
import sys
import re
from utils import (
    analyze_execution_data,
    print_warning,
    round_results,
)

# Configuration constants
PROGRAM_PATH = "./build/src/image-convolution"
IMAGE_DIR = "input_queue_mode"
THREAD_NUM = 2
OUTPUT_DIR = f"tests/plots/queue_mode"
NUM_RUNS = 40
NUM_IMAGES = sys.argv[1]
MEM_LIM = sys.argv[2]

os.makedirs(OUTPUT_DIR, exist_ok=True)

# Configurations for testing (readers, workers, writers)
CONFIGURATIONS = [
    (1, 1, 1),
    (1, 2, 1),
    (1, 3, 1),
    (1, 3, 2),
    (2, 3, 1),
    (2, 3, 2),
    (1, 4, 1),
    (3, 3, 1),
    (2, 4, 2),
]


def run_program(readers: int, workers: int, writers: int, mem_limit: int):
    """
    Runs convolution program in queue mode with specified configuration and memory limit.

    Args:
        readers (int): Number of reader threads.
        workers (int): Number of worker threads.
        writers (int): Number of writer threads.
        mem_limit (int): Memory limit in MiB for the program execution.

    Returns:
        tuple: A tuple containing four NumPy arrays:
            - Total execution times (one per run)
            - Reader execution times (one per image)
            - Worker execution times (one per image)
            - Writer execution times (one per image)
    """
    command = (
        f"{PROGRAM_PATH} {IMAGE_DIR} bl --mode=queue --thread={THREAD_NUM} "
        f"--readers={readers} --workers={workers} --writers={writers} "
        f"--num={NUM_IMAGES} --mem_lim={mem_limit}"
    )

    all_reader_times = []
    all_worker_times = []
    all_writer_times = []
    total_exec_times = []

    for _ in range(NUM_RUNS):
        output = os.popen(command).read()

        reader_times = re.findall(r"READER:.*?in (\d+\.\d+)", output)
        worker_times = re.findall(r"WORKER:.*?in (\d+\.\d+)", output)
        writer_times = re.findall(r"WRITER:.*?in (\d+\.\d+)", output)

        if not reader_times or not worker_times or not writer_times:
            print(f"Failed to parse output")
            continue

        if (
            len(reader_times) != NUM_IMAGES
            or len(worker_times) != NUM_IMAGES
            or len(writer_times) != NUM_IMAGES
        ):
            print(
                f"Incorrect number of records: R:{len(reader_times)}, W:{len(worker_times)}, Wr:{len(writer_times)}"
            )
            continue

        all_reader_times.extend(float(t) for t in reader_times)
        all_worker_times.extend(float(t) for t in worker_times)
        all_writer_times.extend(float(t) for t in writer_times)

        match = re.search(r"The convolution for all images took (\d+\.\d+)", output)
        if match:
            time = float(match.group(1))
            total_exec_times.append(time)
        else:
            print_warning(f"Failed to parse output: {output}")

    return (
        np.array(total_exec_times),
        np.array(all_reader_times),
        np.array(all_worker_times),
        np.array(all_writer_times),
    )


def main():
    """
    The main function organizes the process of comparative analysis of execution time in the queue mode,
    including conducting tests, analyzing results, plotting graphs and saving results.
    """
    data_file = open(f"{OUTPUT_DIR}/benchmark_results_{MEM_LIM}MiB.txt", "w+")

    exec_results = {
        config: {"means": [], "conf_inter": []} for config in CONFIGURATIONS
    }
    reader_results = {
        config: {"means": [], "conf_inter": []} for config in CONFIGURATIONS
    }
    worker_results = {
        config: {"means": [], "conf_inter": []} for config in CONFIGURATIONS
    }
    writer_results = {
        config: {"means": [], "conf_inter": []} for config in CONFIGURATIONS
    }

    data_file.write(f"Memory Limit: {MEM_LIM} MiB\n")

    for config in CONFIGURATIONS:
        data_file.write(
            f"\tConfig: Readers={config[0]} Workers={config[1]} Writers={config[2]}\n"
        )

        readers, workers, writers = config
        print(
            f"Testing configuration: readers={readers}, workers={workers}, writers={writers}, mem_limit={MEM_LIM}"
        )

        exec_times, reader_times, worker_times, writer_times = run_program(
            readers, workers, writers, MEM_LIM
        )

        # Analyze the times for each stage
        def process_stage(res_array, times, filter_data: bool):
            mean_time, confidence_interval = analyze_execution_data(
                times, filter_data
            )
            res_array[config]["means"].append(mean_time)
            res_array[config]["conf_inter"].append(confidence_interval)
            return round_results(mean_time, confidence_interval)

        r_mean, r_error = process_stage(reader_results, reader_times, False)
        w_mean, w_error = process_stage(worker_results, worker_times, False)
        wr_mean, wr_error = process_stage(writer_results, writer_times, False)
        ex_mean, ex_error = process_stage(exec_results, exec_times, True)

        # Save benchmark results to a text file
        data_file.write(
            f"\t\t\tReader result: {r_mean} ± {r_error} s\n"
            f"\t\t\tWorker result: {w_mean} ± {w_error} s\n"
            f"\t\t\tWriter result: {wr_mean} ± {wr_error} s\n"
            f"\t\tExecution result: {ex_mean} ± {ex_error} s\n\n"
        )
    data_file.write("\n")

    # Graph 1: Total execution time for all configurations

    means = [exec_results[c]["means"][0] for c in CONFIGURATIONS]
    errors = [exec_results[c]["conf_inter"][0] for c in CONFIGURATIONS]

    x_pos = np.arange(len(CONFIGURATIONS))
    plt.bar(
        x=x_pos,
        height=means,
        yerr=errors,
        align="center",
        alpha=0.7,
        capsize=10,
        color=(
            [
                "#efa94a",
                "#47a76a",
                "#db5856",
                "#9966cc",
                "#93c6cf",
                "#6c3930",
                "#aec636",
                "#3189e0",
            ]
        ),
    )
    plt.xticks(x_pos, CONFIGURATIONS)
    plt.ylabel("Execution Time (s)")
    plt.title(f"Queue Mode Performance (Mem Limit: {MEM_LIM} MiB)")

    plot_path = os.path.join(
        OUTPUT_DIR, f"queue_performance_comparison_{MEM_LIM}MiB.png"
    )
    plt.tight_layout()
    plt.savefig(plot_path)
    plt.close()

    # Graph 2: Time by stages (reader, worker, writer) for all configurations

    bar_width = 0.15
    stage_colors = {
        "Reader": "#93c6cf",
        "Worker": "#aec636",
        "Writer": "#ae68cf",
    }

    x_pos = np.arange(len(CONFIGURATIONS))
    _, ax = plt.subplots(figsize=(12, 6))

    for i, stage in enumerate(["Reader", "Worker", "Writer"]):
        if stage == "Reader":
            means = [reader_results[c]["means"][0] for c in CONFIGURATIONS]
            errors = [reader_results[c]["conf_inter"][0] for c in CONFIGURATIONS]
        elif stage == "Worker":
            means = [worker_results[c]["means"][0] for c in CONFIGURATIONS]
            errors = [worker_results[c]["conf_inter"][0] for c in CONFIGURATIONS]
        else:
            means = [writer_results[c]["means"][0] for c in CONFIGURATIONS]
            errors = [writer_results[c]["conf_inter"][0] for c in CONFIGURATIONS]

        ax.bar(
            x=(x_pos + i * bar_width),
            height=means,
            yerr=errors,
            width=bar_width,
            label=stage,
            capsize=5,
            color=stage_colors[stage],
            alpha=0.7,
        )

    plt.xticks(x_pos + bar_width)
    ax.set_xticklabels(
        [c for c in CONFIGURATIONS],
        # ha="right",
    )
    plt.ylabel("Execution Time (s)")
    plt.title(f"Stage-wise Performance by Configuration ({MEM_LIM} MiB)")
    plt.legend(title="Stage")

    plot_path_stages = os.path.join(
        OUTPUT_DIR, f"queue_performance_by_stage_{MEM_LIM}MiB.png"
    )
    plt.tight_layout()
    plt.savefig(plot_path_stages)
    plt.close()

    data_file.close()

    print(f"Measurements and charts saved to '{OUTPUT_DIR}' directory.")


if __name__ == "__main__":
    main()
