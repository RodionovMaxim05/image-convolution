import numpy as np
from scipy import stats
from PIL import Image


def get_image_resolution(image_path: str) -> tuple[int, int]:
    """
    Retrieves the resolution (width and height) of an image file.

    Args:
        image_path (str): The file path to the image.

    Returns:
        A tuple (width, height) representing the image resolution in pixels.
    """
    try:
        with Image.open(image_path) as img:
            width, height = img.size
            return width, height
    except Exception as e:
        raise ValueError(f"Error loading {image_path}: {e}")


def print_warning(message: str):
    """
    Prints a formatted warning message in red text to the console.

    Args:
        message (str): The warning message to display.
    """
    RED_ANSI = "\033[91m"
    RESET_ANSI = "\033[0m"
    print(f"{RED_ANSI}*** WARNING ***{RESET_ANSI}")
    print(f"{RED_ANSI}{message}{RESET_ANSI}\n")


def analyze_execution_data(data, filter_data: bool):
    """
    Analyzes execution time data to compute the mean execution time and its confidence interval
    while performing outlier removal and normality testing.

    Args:
        data (array): A list or array of execution times.
        filter_data (bool): If True, applies outlier filtering based on standard deviation.

    Returns:
        A tuple (mean_time, confidence_interval):
            mean_time (float): The mean execution time after filtering outliers.
            confidence_interval (float): The 95% confidence interval for the mean.
    """
    mean_time = np.mean(data)
    std_time = np.std(data, ddof=1)

    if filter_data:
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
    else:
        filtered_data = data

    mean_time = np.mean(filtered_data)
    confidence_interval = stats.t.ppf(0.975, df=len(filtered_data) - 1) * stats.sem(
        filtered_data
    )

    return mean_time, confidence_interval


def round_results(mean_time, confidence_interval):
    """
    Rounds the mean execution time and confidence interval to a consistent number of significant figures.

    Args:
        mean_time (float): Mean execution time.
        confidence_interval (float): Confidence interval for the mean.

    Returns:
        A tuple (rounded_mean, rounded_conf_inter) with appropriately rounded values.
    """
    rounded_conf_inter = round(
        confidence_interval, -int(np.floor(np.log10(abs(confidence_interval))))
    )
    rounded_mean = round(
        mean_time, -int(np.floor(np.log10(abs(rounded_conf_inter))))
    )

    return rounded_mean, rounded_conf_inter
