#include "../src/convolution/filter_application.h"
#include "../src/convolution/parallel_dispatch.h"

#include "utils_for_tests.c"

// Filter Inversion Tests

/**
 * Tests the inversion property of filters by applying two sequential filters
 * (shift_right and shift_left) to a randomly generated image.
 */
static void test_filter_inverse_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	const double shift_right[3][3] = {{0, 0, 0}, {0, 0, 1}, {0, 0, 0}};
	const double shift_left[3][3] = {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}};

	struct filter filter_right = create_filter(3, 1.0, 0.0, shift_right);
	struct filter filter_left = create_filter(3, 1.0, 0.0, shift_left);
	assert_non_null(filter_right.kernel);
	assert_non_null(filter_left.kernel);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter_right);
	sequential_application(&result1, &result2, width, height, filter_left);

	assert_memory_equal(channel_image.red, result2.red, width * height);
	assert_memory_equal(channel_image.green, result2.green, width * height);
	assert_memory_equal(channel_image.blue, result2.blue, width * height);

	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter_right);
	free_filter(&filter_left);
}

/**
 * Tests the inversion property of filters using a predefined default image
 * (cat.bmp).
 */
static void test_filter_inverse_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	const double shift_right[3][3] = {{0, 0, 0}, {0, 0, 1}, {0, 0, 0}};
	const double shift_left[3][3] = {{0, 0, 0}, {1, 0, 0}, {0, 0, 0}};

	struct filter filter_right = create_filter(3, 1.0, 0.0, shift_right);
	struct filter filter_left = create_filter(3, 1.0, 0.0, shift_left);
	assert_non_null(filter_right.kernel);
	assert_non_null(filter_left.kernel);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter_right);
	sequential_application(&result1, &result2, width, height, filter_left);

	assert_memory_equal(channel_image.red, result2.red, width * height);
	assert_memory_equal(channel_image.green, result2.green, width * height);
	assert_memory_equal(channel_image.blue, result2.blue, width * height);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter_right);
	free_filter(&filter_left);
}

// Zero-based Filter Extension Tests

/**
 * Tests the zero-padding functionality by comparing the results of applying a small
 * filter (fast_blur) and its zero-padded version to a randomly generated image.
 */
static void test_filter_zero_padding_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb input_image = create_test_image(width, height);

	struct filter filter = create_filter(3, 1.0, 0.0, fast_blur);
	assert_non_null(filter.kernel);

	struct filter padded_filter = create_filter(
		5, 1.0, 0.0, id); // In fact, it doesn't matter which filter you pass here,
						  // because it will change anyway.
	assert_non_null(padded_filter.kernel);
	apply_zero_padding(&padded_filter, &filter);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);

	sequential_application(&input_image, &result1, width, height, filter);
	sequential_application(&input_image, &result2, width, height, padded_filter);

	assert_memory_equal(result1.red, result2.red, width * height);
	assert_memory_equal(result1.green, result2.green, width * height);
	assert_memory_equal(result1.blue, result2.blue, width * height);

	free_image_rgb(&input_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter);
	free_filter(&padded_filter);
}

/**
 * Tests the zero-padding functionality using a predefined default image (cat.bmp).
 */
static void test_filter_zero_padding_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	struct filter filter = create_filter(3, 1.0, 0.0, fast_blur);
	assert_non_null(filter.kernel);

	struct filter padded_filter = create_filter(5, 1.0, 0.0, id);
	assert_non_null(padded_filter.kernel);
	apply_zero_padding(&padded_filter, &filter);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter);
	sequential_application(&channel_image, &result2, width, height, padded_filter);

	assert_memory_equal(result1.red, result2.red, width * height);
	assert_memory_equal(result1.green, result2.green, width * height);
	assert_memory_equal(result1.blue, result2.blue, width * height);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter);
	free_filter(&padded_filter);
}
