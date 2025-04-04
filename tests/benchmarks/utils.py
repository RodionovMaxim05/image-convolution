import numpy as np
from scipy import stats


def print_warning(message: str):
    RED_ANSI = "\033[91m"
    RESET_ANSI = "\033[0m"
    print(f"{RED_ANSI}*** WARNING ***{RESET_ANSI}")
    print(f"{RED_ANSI}{message}{RESET_ANSI}\n")


def analyze_execution_data(data):
    mean_time = np.mean(data)
    std_time = np.std(data, ddof=1)

    # Removing outliers (greater than 3 standard deviations)
    filtered_data = data[
        (data > mean_time - 3 * std_time) & (data < mean_time + 3 * std_time)
    ]
    if len(data) - len(filtered_data) > 1:
        print_warning("Too many emissions.")

    normal_test = stats.normaltest(filtered_data)
    shapiro_test = stats.shapiro(filtered_data)
    if normal_test.pvalue < 0.05 and shapiro_test.pvalue < 0.05:
        print_warning("Data does not pass normality tests.")

    mean_time = np.mean(filtered_data)
    confidence_interval = stats.t.ppf(0.975, df=len(filtered_data) - 1) * stats.sem(
        filtered_data
    )

    return mean_time, confidence_interval
