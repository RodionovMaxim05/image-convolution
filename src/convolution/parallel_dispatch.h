#include "filter_application.h"

int parallel_pixel(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);

int parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				 int width, int height, struct filter filter, int num_threads);

int parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					int width, int height, struct filter filter, int num_threads);

int parallel_block(struct image_rgb *input_image, struct image_rgb *output_image,
				   int width, int height, struct filter filter, int num_threads);
