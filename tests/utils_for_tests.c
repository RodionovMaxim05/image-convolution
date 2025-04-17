#pragma once

#include "../src/filters/filter.h"
#include "../src/utils/utils.h"

#define UPPER_SIZE_LIMIT 2500 // To ensure that the filter is not used for too long.
#define MAX_RANDOM_FILTER_SIZE 9
#define MIN_FACTOR 0.0005
#define MAX_FACTOR 1.0

/**
 * Initializes an image_rgb structure with the specified dimensions and verifies that
 * memory allocation for its red, green, and blue channels is successful.
 *
 * @param width Image width.
 * @param height Image height.
 *
 * @return An initialized `structure image_rgb`.
 */
static struct image_rgb initialize_and_check_image_rgb(int width, int height) {
	struct image_rgb image = initialize_image_rgb(width, height);
	assert_non_null(image.red);
	assert_non_null(image.green);
	assert_non_null(image.blue);
	return image;
}

/**
 * Creates a random test image with the specified dimensions. Each pixel's red,
 * green, and blue components are assigned random values between 0 and 255.
 *
 * @param width Image width.
 * @param height Image height.
 *
 * @return A `structure image_rgb` representing a randomly generated test image.
 */
static struct image_rgb create_test_image(int width, int height) {
	struct image_rgb image = initialize_and_check_image_rgb(width, height);
	for (size_t i = 0; i < (size_t)width * (size_t)height; i++) {
		image.red[i] = rand() % 256;
		image.green[i] = rand() % 256;
		image.blue[i] = rand() % 256;
	}
	return image;
}

/**
 * Create a random filter with odd size between 3 and MAX_RANDOM_FILTER_SIZE.
 * Core elements >= 0 and bias = 0 to avoid truncating values ​​less than zero
 * and greater than 255.
 *
 * @return A `struct filter` randomly generated filter.
 */
struct filter create_random_filter() {
	struct filter random_filter;

	int random_index = rand() % ((MAX_RANDOM_FILTER_SIZE - 1) / 2);
	int size = 2 * random_index + 3;
	random_filter.size = size;

	random_filter.kernel = (double **)malloc(size * sizeof(double *));
	for (int i = 0; i < size; i++) {
		random_filter.kernel[i] = (double *)malloc(size * sizeof(double));
	}

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			random_filter.kernel[i][j] = (double)rand();
		}
	}

	random_filter.factor =
		MIN_FACTOR + ((double)rand() / RAND_MAX) * (MAX_FACTOR - MIN_FACTOR);
	random_filter.bias = 0;

	return random_filter;
}

/**
 * Compares two images channel by channel for exact equality.
 *
 * @param expected Pointer to the expected image.
 * @param actual Pointer to the actual image.
 * @param width Image width.
 * @param height Image height.
 *
 * @return true if channels are identical, false otherwise.
 */
bool compare_channels(struct image_rgb *expected, struct image_rgb *actual,
					  int width, int height) {
	for (size_t i = 0; i < (size_t)width * (size_t)height; i++) {
		if (expected->red[i] != actual->red[i] ||
			expected->green[i] != actual->green[i] ||
			expected->blue[i] != actual->blue[i]) {
			return false;
		}
	}
	return true;
}

/**
 * Compares two images channel by channel with a tolerance of 1.
 *
 * @param expected Pointer to the expected image.
 * @param actual Pointer to the actual image.
 * @param width Image width.
 * @param height Image height.
 *
 * @return true if channels are similar, false otherwise.
 */
bool compare_channels_with_epsilon(struct image_rgb *expected,
								   struct image_rgb *actual, int width, int height) {
	for (size_t i = 0; i < (size_t)width * (size_t)height; i++) {
		if (expected->red[i] - actual->red[i] > 1 ||
			expected->green[i] - actual->green[i] > 1 ||
			expected->blue[i] - actual->blue[i] > 1) {
			return false;
		}
	}
	return true;
}

/**
 * Composes two filters into a single filter.
 *
 * @param filter1 First filter.
 * @param filter2 Second filter.
 *
 * @return A `struct filter` Composed filter
 */
struct filter compose_filters(struct filter filter1, struct filter filter2) {
	int new_size = filter1.size + filter2.size - 1;

	double **kernel = (double **)malloc(new_size * sizeof(double *));
	for (int i = 0; i < new_size; i++) {
		kernel[i] = (double *)calloc(new_size, sizeof(double));
	}

	for (int i = 0; i < new_size; i++) {
		for (int j = 0; j < new_size; j++) {

			for (int fi = 0; fi < filter1.size; fi++) {
				for (int fj = 0; fj < filter1.size; fj++) {
					int i2 = i - fi;
					int j2 = j - fj;

					if (i2 >= 0 && i2 < filter2.size && j2 >= 0 &&
						j2 < filter2.size) {
						kernel[i][j] +=
							filter1.kernel[fi][fj] * filter2.kernel[i2][j2];
					}
				}
			}
		}
	}

	double new_factor = filter1.factor * filter2.factor;
	double new_bias = filter1.bias * filter2.factor + filter2.bias;

	struct filter composed = {
		.size = new_size, .kernel = kernel, .factor = new_factor, .bias = new_bias};

	return composed;
}

/**
 * Print filter details for debugging.
 *
 * @param filter Pointer to the filter.
 * @param name Name of the filter.
 */
void print_filter(const struct filter *filter, const char *name) {
	printf("%s:\n", name);
	printf("{\n");
	for (int i = 0; i < filter->size; i++) {
		printf("  {");
		for (int j = 0; j < filter->size; j++) {
			printf("%.1f", filter->kernel[i][j]);
			if (j != filter->size - 1) {
				printf(", ");
			}
		}
		printf("}");
		if (i != filter->size - 1) {
			printf(",");
		}
		printf("\n");
	}
	printf("}\n");
	printf("factor: %.4f\n", filter->factor);
	printf("bias: %.1f\n", filter->bias);
	printf("\n");
}

/**
 * Applies zero padding to a convolution filter by embedding the original filter's
 * kernel into the center of a larger kernel filled with zeros.
 *
 * @param padded_filter Pointer to the filter that will receive the zero-padded
 * kernel.
 * @param original_filter Pointer to the original filter whose kernel will be
 * embedded.
 */
static void apply_zero_padding(struct filter *padded_filter,
							   struct filter *original_filter) {
	// Filling the extended filter kernel with zeros
	for (int i = 0; i < padded_filter->size; i++) {
		for (int j = 0; j < padded_filter->size; j++) {
			padded_filter->kernel[i][j] = 0.0;
		}
	}

	// Copying the original filter to the center of the advanced filter
	int offset = (padded_filter->size - original_filter->size) / 2;
	for (int i = 0; i < original_filter->size; i++) {
		for (int j = 0; j < original_filter->size; j++) {
			padded_filter->kernel[i + offset][j + offset] =
				original_filter->kernel[i][j];
		}
	}
}

/**
 * Save an image for debugging failed tests.
 *
 * @param image Image to save.
 * @param width Image width.
 * @param height Image height.
 * @param test_name Name of the test (used for filename).
 */
void save_image(struct image_rgb image, int width, int height, char *test_name) {
	unsigned char *result_image = malloc((size_t)width * (size_t)height * (size_t)3);
	assert_non_null(result_image);

	assemble_image_from_rgb_channels(result_image, image, width, height);

	char *output_file_path = malloc(strlen(test_name) + 1);
	assert_non_null(output_file_path);
	sprintf(output_file_path, "%s", test_name);

	int success = stbi_write_bmp(output_file_path, width, height, 3, result_image);

	printf("The image on which the test failed is located at 'build/tests/%s'\n",
		   output_file_path);
}
