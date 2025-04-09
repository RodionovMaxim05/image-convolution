import numpy as np
from scipy import stats
from PIL import Image


def get_image_resolution(image_path: str) -> tuple[int, int]:
    """Retrieves the resolution (width and height) of an image file."""
    try:
        with Image.open(image_path) as img:
            width, height = img.size
            return width, height
    except Exception as e:
        raise ValueError(f"Error loading {image_path}: {e}")


def print_warning(message: str):
    """Prints a formatted warning message in red text to the console."""
    RED_ANSI = "\033[91m"
    RESET_ANSI = "\033[0m"
    print(f"{RED_ANSI}*** WARNING ***{RESET_ANSI}")
    print(f"{RED_ANSI}{message}{RESET_ANSI}\n")


def analyze_execution_data(data):
    """
    Analyzes execution time data to compute the mean execution time and its confidence interval
    while performing outlier removal and normality testing.
    """
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
