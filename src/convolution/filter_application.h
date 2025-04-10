#pragma once

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#include "../utils/utils.h"

#include "../filters/filter.h"

/**
 * Represents the data passed to each thread during parallel filter application.
 *
 * @param input_image Pointer to the input image (RGB channels) to be processed.
 * @param output_image Pointer to the output image (RGB channels) where the result
 * will be stored.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 * @param block_width Width of each processing block in the image.
 * @param block_height Height of each processing block in the image.
 * @param num_cols Number of columns of blocks in the image.
 * @param num_blocks Total number of blocks in the image.
 * @param next_block Atomic integer pointer used to assign blocks dynamically to
 * threads.
 */
struct thread_data {
	struct image_rgb *input_image;
	struct image_rgb *output_image;
	int width;
	int height;
	struct filter filter;
	int block_width;
	int block_height;
	int num_cols;
	int num_blocks;
	atomic_int *next_block;
};

/**
 * Applies a convolution filter sequentially to an image.
 *
 * @param input_image Pointer to the input image (`struct image_rgb`).
 * @param output_image Pointer to the output image (`struct image_rgb`).
 * @param width Width of the image.
 * @param height Height of the image.
 * @param filter The convolution filter to be applied.
 */
void sequential_application(struct image_rgb *input_image,
							struct image_rgb *output_image, int width, int height,
							struct filter filter);

/**
 * Processes image blocks dynamically in a parallel execution environment.
 *
 * @param arg A pointer to a `struct thread_data` containing the thread's processing
 * data.
 */
void *process_dynamic(void *arg);
