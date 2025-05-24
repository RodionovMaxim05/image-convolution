#include "../src/convolution/filter_application.h"

#include "utils_for_tests.h"

#define IMAGE_WIDTH 2
#define IMAGE_HEIGHT 2

unsigned char test_image[] = {
	255, 0, 0,	 0,	  255, 0,  // red green
	0,	 0, 255, 255, 255, 255 // blue white
};

/**
 * Tests the creation of a convolution filter using the `create_filter()` function.
 */
void test_create_filter(void **state) {
	(void)state;

	struct filter filter = create_filter(3, 1.0, 0.0, id);
	assert_non_null(filter.kernel);

	assert_int_equal(filter.size, 3);
	assert_double_equal(filter.factor, 1.0, 1e-6);
	assert_double_equal(filter.bias, 0.0, 1e-6);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			assert_double_equal(filter.kernel[i][j], id[i][j], 1e-6);
		}
	}

	free_filter(&filter);
}

/**
 * Tests the splitting of an image into RGB channels
 * (`split_image_into_rgb_channels()`) and reassembling it back into a single image
 * (`assemble_image_from_rgb_channels()`).
 */
void test_split_assemble_channels(void **state) {
	(void)state;

	struct image_rgb channel_image =
		initialize_and_check_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);
	split_image_into_rgb_channels(test_image, channel_image, IMAGE_WIDTH,
								  IMAGE_HEIGHT);

	unsigned char expected_red[] = {255, 0, 0, 255};
	unsigned char expected_green[] = {0, 255, 0, 255};
	unsigned char expected_blue[] = {0, 0, 255, 255};

	assert_memory_equal(channel_image.red, expected_red, IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.green, expected_green,
						IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.blue, expected_blue,
						IMAGE_WIDTH * IMAGE_HEIGHT);

	unsigned char *assembled_image = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3);
	assert_non_null(assembled_image);
	assemble_image_from_rgb_channels(assembled_image, channel_image, IMAGE_WIDTH,
									 IMAGE_HEIGHT);

	assert_memory_equal(test_image, assembled_image, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

	free(assembled_image);
	free_image_rgb(&channel_image);
}

/**
 * Tests the application of an identity filter to an image.
 */
void test_identity_filter(void **state) {
	(void)state;

	struct image_rgb channel_image =
		initialize_and_check_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);
	struct image_rgb result_channel_image =
		initialize_and_check_image_rgb(IMAGE_WIDTH, IMAGE_HEIGHT);

	split_image_into_rgb_channels(test_image, channel_image, IMAGE_WIDTH,
								  IMAGE_HEIGHT);

	double identity_kernel[3][3] = {{0, 0, 0}, {0, 1, 0}, {0, 0, 0}};
	struct filter filter = create_filter(3, 1.0, 0.0, identity_kernel);
	assert_non_null(filter.kernel);

	sequential_application(&channel_image, &result_channel_image, IMAGE_WIDTH,
						   IMAGE_HEIGHT, filter);

	assert_memory_equal(channel_image.red, result_channel_image.red,
						IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.green, result_channel_image.green,
						IMAGE_WIDTH * IMAGE_HEIGHT);
	assert_memory_equal(channel_image.blue, result_channel_image.blue,
						IMAGE_WIDTH * IMAGE_HEIGHT);

	free_filter(&filter);
	free_image_rgb(&channel_image);
	free_image_rgb(&result_channel_image);
}

int main(void) {
	// Initialize random number generator with current time
	srand((unsigned int)time(NULL));

	const struct CMUnitTest core_tests[] = {
		cmocka_unit_test(test_create_filter),
		cmocka_unit_test(test_split_assemble_channels),
		cmocka_unit_test(test_identity_filter),
	};

	return cmocka_run_group_tests_name("Core Functionality Tests", core_tests, NULL,
									   NULL);
}
