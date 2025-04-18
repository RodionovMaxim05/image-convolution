[//]: # (Project readme template from https://github.com/othneildrew/Best-README-Template/)
<a name="readme-top"></a>

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)


<h1 align="left">Image-convolution</h1>

## Description

Image-convolution is an image processing application that applies various convolution filters to images. It supports both sequential and parallel execution modes with different workload distribution strategies. The application is designed for benchmarking and comparing different parallelization approaches for image processing algorithms.

## Usage
Basic Command:
```bash
./build/src/image-convolution <image_path> <filter_name> --mode=<mode> [--thread=<num>]
```
### Options
| Parameter          | Description                                                         | Required |
|--------------------|---------------------------------------------------------------------|----------|
| `<image_path>`     | Path to input image or `--default-image` (predefined default image) | Yes      |
| `<filter_name>`    | Filter to apply (see [Available Filters](#available-filters))       | Yes      |
| `--mode=<mode>`    | Execution mode: `seq`, `pixel`, `row`, `column`, `block`            | Yes      |
| `--thread=<num>`   | Number of threads (for parallel modes)                              | No*      |

*Required for all modes except seq

### Available Filters
| Name      | Description                                            | Kernel Size |
|-----------|--------------------------------------------------------|-------------|
| `id`      | Identity filter (no effect)                            | 3x3         |
| `fbl`     | Fast blur filter                                       | 3x3         |
| `bl`      | Standard blur filter                                   | 5x5         |
| `gbl`     | Gaussian blur filter                                   | 5x5         |
| `mbl`     | Motion blur filter                                     | 9x9         |
| `ed`      | Edge detection filter                                  | 3x3         |
| `em`      | Emboss filter                                          | 5x5         |
| `bl+gbl`  | Composition of Standard blur and Gaussian blur filters | 9x9         |
| `fbl+mbl` | Composition of Fast blur and Motion blur filters       | 11x11       |

### Examples
1) Sequential processing:
```bash
./build/src/image-convolution images/cat.bmp bl --mode=seq
```
2) Parallel processing (4 threads):
```bash
./build/src/image-convolution images/cat.bmp gbl --mode=block --thread=4
```

## Build
To build the project:
```bash
./scripts/build.sh --release
```

## Testing
The project includes three types of tests:
1) Unit tests - verify core functionality:
```bash
./scripts/test.sh
```
2) Performance benchmarks - measure execution times for different filters and modes:
```bash
./scripts/time_benchmark.sh <image_name>
```
3) Cache performance analysis - analyze cache behavior using `perf`:
```bash
./scripts/perf.sh <image_name>
```

## Prerequisites for Benchmarks (`Performance benchmarks` and `Cache performance analysis`)
Before running benchmarks, you need to set up a Python virtual environment and install dependencies:
1. Create a Python virtual environment:
```bash
python3 -m venv .venv
```
2. Activate the virtual environment:
```bash
source .venv/bin/activate
```
3. Install required dependencies:
```bash
pip install -r requirements.txt
```
Note: The image for benchmarks must be in the 'images' directory

## License

Distributed under the [MIT License](https://choosealicense.com/licenses/mit/). See [`LICENSE`](LICENSE) for more
information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>
