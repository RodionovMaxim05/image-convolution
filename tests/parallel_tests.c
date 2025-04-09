#include "../src/convolution/filter_application.h"
#include "../src/convolution/parallel_dispatch.h"

#include "utils_for_tests.c"

/**
 * A helper function that runs a parallel test for a given parallel implementation
 * (parallel_function). It compares the results of the parallel implementation with
 * those of the sequential implementation to ensure correctness.
 */
static void run_test_with_filter(
	int (*parallel_function)(struct image_rgb *, struct image_rgb *, int, int,
							 struct filter, int), // a little bit fp 🫠
	struct image_rgb *channel_image, int width, int height, int num_threads) {

	struct filter filter = create_filter(9, 1.0 / 9.0, 0.0, motion_blur);
	assert_non_null(filter.kernel);

	struct image_rgb result_seq = initialize_and_check_image_rgb(width, height);
	struct image_rgb result_par = initialize_and_check_image_rgb(width, height);

	sequential_application(channel_image, &result_seq, width, height, filter);
	assert_int_equal(parallel_function(channel_image, &result_par, width, height,
									   filter, num_threads),
					 0);

	assert_memory_equal(result_seq.red, result_par.red, width * height);
	assert_memory_equal(result_seq.green, result_par.green, width * height);
	assert_memory_equal(result_seq.blue, result_par.blue, width * height);

	free_image_rgb(channel_image);
	free_image_rgb(&result_seq);
	free_image_rgb(&result_par);
	free_filter(&filter);
}

/**
 * Tests the `parallel_pixel()` implementation using a predefined default image
 * (cat.bmp).
 */
static void test_parallel_pixel_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	run_test_with_filter(parallel_pixel, &channel_image, width, height, 3);

	stbi_image_free(image);
}

/**
 * Tests the `parallel_pixel()` implementation using a randomly generated image.
 */
static void test_parallel_pixel_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	run_test_with_filter(parallel_pixel, &channel_image, width, height, 3);
}

/**
 * Tests the `parallel_row()` implementation using a predefined default image
 * (cat.bmp).
 */
static void test_parallel_row_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	run_test_with_filter(parallel_row, &channel_image, width, height, 3);

	stbi_image_free(image);
}

/**
 * Tests the `parallel_row()` implementation using a randomly generated image
 */
static void test_parallel_row_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	run_test_with_filter(parallel_row, &channel_image, width, height, 3);
}

/**
 * Tests the `parallel_column()` implementation using a predefined default image
 * (cat.bmp).
 */
static void test_parallel_column_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	run_test_with_filter(parallel_column, &channel_image, width, height, 3);

	stbi_image_free(image);
}

/**
 * Tests the `parallel_column()` implementation using a randomly generated image.
 */
static void test_parallel_column_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	run_test_with_filter(parallel_column, &channel_image, width, height, 3);
}

/**
 * Tests the `parallel_block()` implementation using a predefined default image
 * (cat.bmp).
 */
static void test_parallel_block_with_default_image(void **state) {
	(void)state;

	int width, height, channels;
	unsigned char *image =
		stbi_load("../../images/cat.bmp", &width, &height, &channels, 3);
	assert_true(image);

	struct image_rgb channel_image = initialize_and_check_image_rgb(width, height);
	split_image_into_rgb_channels(image, channel_image, width, height);

	run_test_with_filter(parallel_block, &channel_image, width, height, 3);

	stbi_image_free(image);
}

/**
 * Tests the `parallel_block()` implementation using a randomly generated image.
 */
static void test_parallel_block_with_random_image(void **state) {
	(void)state;

	int width = (rand() % UPPER_SIZE_LIMIT), height = (rand() % UPPER_SIZE_LIMIT);
	printf("Testing with random image size: %d x %d\n", width, height);

	struct image_rgb channel_image = create_test_image(width, height);

	run_test_with_filter(parallel_block, &channel_image, width, height, 3);
}
