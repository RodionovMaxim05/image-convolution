#pragma once

#include "../src/filters/filter.h"
#include "../src/utils/utils.h"

static struct image_rgb initialize_and_check_image_rgb(int width, int height) {
	struct image_rgb image = initialize_image_rgb(width, height);
	assert_non_null(image.red);
	assert_non_null(image.green);
	assert_non_null(image.blue);
	return image;
}

static struct image_rgb create_test_image(int width, int height) {
	struct image_rgb image = initialize_and_check_image_rgb(width, height);
	for (int i = 0; i < width * height; i++) {
		image.red[i] = rand() % 256;
		image.green[i] = rand() % 256;
		image.blue[i] = rand() % 256;
	}
	return image;
}

static void apply_zero_padding(struct filter *padded_filter,
							   const struct filter *original_filter) {
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
