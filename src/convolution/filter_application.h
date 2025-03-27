#pragma once

#include <pthread.h>
#include <stdio.h>

#include "../filters/filter.h"
#include "../utils/utils.h"

struct thread_data {
	struct image_rgb *input_image;
	struct image_rgb *output_image;
	int width;
	int height;
	struct filter filter;
	int start_x;
	int start_y;
	int end_x;
	int end_y;
};

void sequential_application(struct image_rgb *input_image,
							struct image_rgb *output_image, int width, int height,
							struct filter filter);

void *process_image_part(void *arg);
