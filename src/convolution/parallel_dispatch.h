#include "filter_application.h"

/**
 * Applies a convolution filter to an image in parallel by processing individual
 * pixels.
 *
 * @param input_image Pointer to the input image (`struct image_rgb`).
 * @param output_image Pointer to the output image (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 * @param num_threads Number of threads to use for parallel processing.
 *
 * @return `0` on success. `-1` if thread creation fails.
 */
int parallel_pixel(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing rows of pixels.
 *
 * @param input_image Pointer to the input image (`struct image_rgb`).
 * @param output_image Pointer to the output image (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 * @param num_threads Number of threads to use for parallel processing.
 *
 * @return `0` on success. `-1` if thread creation fails.
 */
int parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				 int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing columns of
 * pixels.
 *
 * @param input_image Pointer to the input image (`struct image_rgb`).
 * @param output_image Pointer to the output image (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 * @param num_threads Number of threads to use for parallel processing.
 *
 * @return `0` on success. `-1` if thread creation fails.
 */
int parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					int width, int height, struct filter filter, int num_threads);

/**
 * Applies a convolution filter to an image in parallel by processing blocks of
 * pixels.
 *
 * @param input_image Pointer to the input image (`struct image_rgb`).
 * @param output_image Pointer to the output image (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 * @param num_threads Number of threads to use for parallel processing.
 *
 * @return `0` on success. `-1` if thread creation fails.
 */
int parallel_block(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);
