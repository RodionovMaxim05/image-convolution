#pragma once

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <assert.h>
#include <cmocka.h>
#include <time.h>

#include "../stb_image/stb_image.h"
#include "../stb_image/stb_image_write.h"

#define UPPER_SIZE_LIMIT 2500 // To ensure that the filter is not used for too long.
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
struct image_rgb initialize_and_check_image_rgb(int width, int height);

/**
 * Creates a random test image with the specified dimensions. Each pixel's red,
 * green, and blue components are assigned random values between 0 and 255.
 *
 * @param width Image width.
 * @param height Image height.
 *
 * @return A `structure image_rgb` representing a randomly generated test image.
 */
struct image_rgb create_test_image(int width, int height);

/**
 * Create a random filter.
 * Core elements >= 0 and bias = 0 to avoid truncating values less than zero and
 * greater than 255.
 *
 * @param size The size of the filter kernel.
 * @param kernel A 2D array representing the kernel matrix. The function populates
 * this array with random values.
 *
 * @return A `struct filter` randomly generated filter.
 */
struct filter generate_random_filter(int size, double kernel[size][size]);

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
					  int width, int height);

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
								   struct image_rgb *actual, int width, int height);

/**
 * Print filter details for debugging.
 *
 * @param filter Pointer to the filter.
 * @param name Name of the filter.
 */
void print_filter(const struct filter *filter, const char *name);

/**
 * Applies zero padding to a convolution filter by embedding the original
 * filter's kernel into the center of a larger kernel filled with zeros.
 *
 * @param padded_filter Pointer to the filter that will receive the zero-padded
 * kernel.
 * @param original_filter Pointer to the original filter whose kernel will be
 * embedded.
 */
void apply_zero_padding(struct filter *padded_filter,
						struct filter *original_filter);

/**
 * Save an image for debugging failed tests.
 *
 * @param image Image to save.
 * @param width Image width.
 * @param height Image height.
 * @param test_name Name of the test (used for filename).
 */
void save_image(struct image_rgb image, int width, int height, char *test_name);
