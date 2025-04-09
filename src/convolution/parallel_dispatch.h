#include "filter_application.h"

/**
 * Applies a convolution filter to an image in parallel by processing individual
 * pixels.
 */
int parallel_pixel(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing rows of pixels.
 */
int parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				 int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing columns of
 * pixels.
 */
int parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing blocks of
 * pixels.
 */
int parallel_block(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);
