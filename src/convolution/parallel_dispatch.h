#include "filter_application.h"

void parallel_row(struct image_rgb *input_image, struct image_rgb *output_image,
				  int width, int height, struct filter filter, int num_threads);

void parallel_column(struct image_rgb *input_image, struct image_rgb *output_image,
					 int width, int height, struct filter filter, int num_threads);
