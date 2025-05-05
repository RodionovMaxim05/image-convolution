#include "../src/filters/filter.h"
#include "../src/utils/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "utils_for_tests.h"

struct image_rgb initialize_and_check_image_rgb(int width, int height) {
	struct image_rgb image = initialize_image_rgb(width, height);
	assert_non_null(image.red);
	assert_non_null(image.green);
	assert_non_null(image.blue);
	return image;
}

struct image_rgb create_test_image(int width, int height) {
	struct image_rgb image = initialize_and_check_image_rgb(width, height);
	for (size_t i = 0; i < (size_t)width * (size_t)height; i++) {
		image.red[i] = rand() % 256;
		image.green[i] = rand() % 256;
		image.blue[i] = rand() % 256;
	}
	return image;
}

struct filter generate_random_filter(int size, double kernel[size][size]) {
	struct filter random_filter;
	random_filter.size = size;

	random_filter.kernel = (double **)malloc(size * sizeof(double *));
	assert_non_null(random_filter.kernel);

	for (int i = 0; i < size; i++) {
		random_filter.kernel[i] = (double *)malloc(size * sizeof(double));
		assert_non_null(random_filter.kernel[i]);

		for (int j = 0; j < size; j++) {
			kernel[i][j] = (double)rand();
			random_filter.kernel[i][j] = kernel[i][j];
		}
	}

	random_filter.factor =
		MIN_FACTOR + ((double)rand() / RAND_MAX) * (MAX_FACTOR - MIN_FACTOR);
	random_filter.bias = 0;

	return random_filter;
}

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
	printf("factor: %f\n", filter->factor);
	printf("bias: %.1f\n", filter->bias);
	printf("\n");
}

void apply_zero_padding(struct filter *padded_filter,
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
