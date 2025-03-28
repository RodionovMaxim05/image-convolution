#pragma once

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#include "../utils/utils.h"

#include "../filters/filter.h"

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

void sequential_application(struct image_rgb *input_image,
							struct image_rgb *output_image, int width, int height,
							struct filter filter);

void *process_dynamic(void *arg);
