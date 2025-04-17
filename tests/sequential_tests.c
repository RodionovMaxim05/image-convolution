#include "../src/convolution/filter_application.h"
#include "../src/convolution/parallel_dispatch.h"

#include "utils_for_tests.c"

// Filter composition tests

/**
 * Tests the composition of two random filters applied sequentially
 * versus their composed version on a predefined default image (cat.bmp).
 */
static void test_filter_compose_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	struct filter filter_1 = create_random_filter();
	struct filter filter_2 = create_random_filter();
	assert_non_null(filter_1.kernel);
	assert_non_null(filter_2.kernel);
	printf("Testing with random filters size: %d and %d\n", filter_1.size,
		   filter_2.size);

	struct filter composed_filter = compose_filters(filter_2, filter_1);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result_composed = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter_1);
	sequential_application(&result1, &result2, width, height, filter_2);

	sequential_application(&channel_image, &result_composed, width, height,
						   composed_filter);

	if (!compare_channels_with_epsilon(&result_composed, &result2, width, height)) {
		printf("The images are not similar!\n");

		print_filter(&filter_1, "FILTER 1");
		print_filter(&filter_2, "FILTER 2");
		fflush(stdout);

		assert(false);
	}

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_image_rgb(&result_composed);
	free_filter(&filter_1);
	free_filter(&filter_2);
	free_filter(&composed_filter);
}

/**
 * Tests the composition of two random filters applied sequentially
 * versus their composed version on a randomly generated image.
 */
static void test_filter_compose_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	struct filter filter_1 = create_random_filter();
	struct filter filter_2 = create_random_filter();
	assert_non_null(filter_1.kernel);
	assert_non_null(filter_2.kernel);
	printf("Testing with random filters size: %d and %d\n", filter_1.size,
		   filter_2.size);

	struct filter composed_filter = compose_filters(filter_2, filter_1);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result_composed = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter_1);
	sequential_application(&result1, &result2, width, height, filter_2);
	sequential_application(&channel_image, &result_composed, width, height,
						   composed_filter);

	if (!compare_channels_with_epsilon(&result_composed, &result2, width, height)) {
		printf("The images are not similar!\n");
		save_image(channel_image, width, height,
				   "test_filter_compose_with_random_image.bmp");

		print_filter(&filter_1, "FILTER 1");
		print_filter(&filter_2, "FILTER 2");
		fflush(stdout);

		assert(false);
	}

	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_image_rgb(&result_composed);
	free_filter(&filter_1);
	free_filter(&filter_2);
	free_filter(&composed_filter);
}

// Filter Inversion Tests

/**
 * Tests the inversion property of filters using a predefined
 * default image (cat.bmp).
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

	assert_memory_equal(channel_image.red, result2.red,
						(size_t)width * (size_t)height);
	assert_memory_equal(channel_image.green, result2.green,
						(size_t)width * (size_t)height);
	assert_memory_equal(channel_image.blue, result2.blue,
						(size_t)width * (size_t)height);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter_right);
	free_filter(&filter_left);
}

/**
 * Tests the inversion property of filters by applying two
 * sequential filters (shift_right and shift_left) to a randomly
 * generated image.
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

	if (!compare_channels_with_epsilon(&channel_image, &result2, width, height)) {
		printf("The images are not similar!\n");
		save_image(channel_image, width, height,
				   "test_filter_inverse_with_random_image.bmp");
		fflush(stdout);

		assert(false);
	}

	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter_right);
	free_filter(&filter_left);
}

// Zero-based Filter Extension Tests

/**
 * Tests the zero-padding functionality using a predefined
 * default image (cat.bmp).
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

	assert_memory_equal(result1.red, result2.red, (size_t)width * (size_t)height);
	assert_memory_equal(result1.green, result2.green,
						(size_t)width * (size_t)height);
	assert_memory_equal(result1.blue, result2.blue, (size_t)width * (size_t)height);

	stbi_image_free(image);
	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter);
	free_filter(&padded_filter);
}

/**
 * Tests the zero-padding functionality by comparing the results
 * of applying a small filter (fast_blur) and its zero-padded
 * version to a randomly generated image.
 */
static void test_filter_zero_padding_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	struct filter filter = create_filter(3, 1.0, 0.0, fast_blur);
	assert_non_null(filter.kernel);

	struct filter padded_filter =
		create_filter(5, 1.0, 0.0,
					  id); // In fact, it doesn't matter which filter you pass
						   // here, because it will change anyway.
	assert_non_null(padded_filter.kernel);
	apply_zero_padding(&padded_filter, &filter);

	struct image_rgb result1 = initialize_and_check_image_rgb(width, height);
	struct image_rgb result2 = initialize_and_check_image_rgb(width, height);

	sequential_application(&channel_image, &result1, width, height, filter);
	sequential_application(&channel_image, &result2, width, height, padded_filter);

	if (!compare_channels_with_epsilon(&result1, &result2, width, height)) {
		printf("The images are not similar!\n");
		save_image(channel_image, width, height,
				   "test_filter_zero_padding_with_random_image.bmp");
		fflush(stdout);

		assert(false);
	}

	free_image_rgb(&channel_image);
	free_image_rgb(&result1);
	free_image_rgb(&result2);
	free_filter(&filter);
	free_filter(&padded_filter);
}
