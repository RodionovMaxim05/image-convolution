#pragma once

#include <stdlib.h>

#define ID_SIZE 3
#define ID_FACTOR 1.0
#define ID_BIAS 0.0

#define FAST_BLUR_SIZE 3
#define FAST_BLUR_FACTOR 1.0
#define FAST_BLUR_BIAS 0.0

#define BLUR_SIZE 5
#define BLUR_FACTOR (1.0 / 13.0)
#define BLUR_BIAS 0.0

#define GAUS_BLUR_SIZE 5
#define GAUS_BLUR_FACTOR (1.0 / 256.0)
#define GAUS_BLUR_BIAS 0.0

#define MOTION_BLUR_SIZE 9
#define MOTION_BLUR_FACTOR (1.0 / 9.0)
#define MOTION_BLUR_BIAS 0.0

#define EDGE_DETECTION_SIZE 3
#define EDGE_DETECTION_FACTOR 1.0
#define EDGE_DETECTION_BIAS 0.0

#define EMBOSS_SIZE 5
#define EMBOSS_FACTOR 1.0
#define EMBOSS_BIAS 128.0

#define NUM_OF_FILTERS 9

/**
 * Represents metadata about a filter, including its name and description.
 *
 * @param name The name of the filter (e.g., "id", "fbl").
 * @param description A brief description of the filter's purpose.
 */
typedef struct {
	const char *name;
	const char *description;
} FilterInfo;

extern const FilterInfo filters_info[];

/**
 * Represents a convolution filter with its size, scaling factor, bias, and kernel.
 *
 * @param size Size of the filter kernel (e.g., 3 for a 3x3 kernel).
 * @param factor Scaling factor applied to the convolution result.
 * @param bias Bias added to the convolution result.
 * @param kernel Pointer to a dynamically allocated 2D array representing the filter
 * kernel.
 */
struct filter {
	int size;
	double factor;
	double bias;
	double **kernel;
};

extern const double id[3][3];
extern const double fast_blur[3][3];
extern const double blur[5][5];
extern const double gaus_blur[5][5];
extern const double motion_blur[9][9];
extern const double edge_detection[3][3];
extern const double emboss[5][5];

/**
 * Creates a convolution filter with the specified size, factor, bias, and kernel
 * values.
 *
 * @param size Size of the filter kernel (e.g., 3 for a 3x3 kernel).
 * @param factor Scaling factor for the convolution result.
 * @param bias Bias added to the convolution result.
 * @param kernel: A 2D array containing the kernel values.
 *
 * @return A `struct filter` with the specified properties. If memory allocation
 * fails, returns an empty filter `{0, 0.0, 0.0, NULL}`.
 */
struct filter create_filter(int size, double factor, double bias,
							const double values[size][size]);

/**
 * Frees the memory allocated for a filter's kernel.
 *
 * @param filter Pointer to the `struct filter` whose memory needs to be freed.
 */
void free_filter(struct filter *f);

/**
 * Composes two convolution filters into a single filter by combining their kernels,
 * scaling factors, and biases.
 *
 * @param size1 Size of the first filter's kernel.
 * @param factor1 Scaling factor of the first filter.
 * @param bias1 Bias value of the first filter.
 * @param kernel1 Kernel matrix of the first filter.
 * @param size2 Size of the second filter's kernel.
 * @param factor2 Scaling factor of the second filter.
 * @param bias2 Bias value of the second filter.
 * @param kernel2 Kernel matrix of the second filter.
 *
 * @return A `struct filter` with the composed properties. If memory allocation
 * fails, returns an empty filter `{0, 0.0, 0.0, NULL}`.
 */
struct filter compose_filters_from_params(int size1, double factor1, double bias1,
										  const double kernel1[size1][size1],
										  int size2, double factor2, double bias2,
										  const double kernel2[size2][size2]);
