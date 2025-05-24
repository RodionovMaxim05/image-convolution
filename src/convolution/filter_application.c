#include "filter_application.h"

// Source: https://lodev.org/cgtutor/filtering.html
void sequential_application(struct image_rgb *input_image,
							struct image_rgb *output_image, int width, int height,
							struct filter filter) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			double red = 0.0, green = 0.0, blue = 0.0;

			// multiply every value of the filter with corresponding image pixel
			for (int filterY = 0; filterY < filter.size; filterY++) {
				for (int filterX = 0; filterX < filter.size; filterX++) {
					int imageX = (x - filter.size / 2 + filterX + width) % width;
					int imageY = (y - filter.size / 2 + filterY + height) % height;

					red += input_image->red[imageY * width + imageX] *
						   filter.kernel[filterY][filterX];
					green += input_image->green[imageY * width + imageX] *
							 filter.kernel[filterY][filterX];
					blue += input_image->blue[imageY * width + imageX] *
							filter.kernel[filterY][filterX];
				}
			}

			// truncate values smaller than zero and larger than 255
			output_image->red[y * width + x] =
				min(max((int)(filter.factor * red + filter.bias), 0), 255);
			output_image->green[y * width + x] =
				min(max((int)(filter.factor * green + filter.bias), 0), 255);
			output_image->blue[y * width + x] =
				min(max((int)(filter.factor * blue + filter.bias), 0), 255);
		}
	}
}

void *process_dynamic(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;

	while (1) {
		int block_index = atomic_fetch_add(data->next_block, 1);

		if (block_index >= data->num_blocks) {
			break;
		}

		size_t block_x = block_index % data->num_cols;
		size_t block_y = block_index / data->num_cols;

		size_t start_x = block_x * data->block_width;
		size_t start_y = block_y * data->block_height;

		size_t end_x = min(start_x + data->block_width, (size_t)data->width);
		size_t end_y = min(start_y + data->block_height, (size_t)data->height);

		for (size_t y = start_y; y < end_y; y++) {
			for (size_t x = start_x; x < end_x; x++) {
				double red = 0.0, green = 0.0, blue = 0.0;

				for (int filterY = 0; filterY < data->filter.size; filterY++) {
					for (int filterX = 0; filterX < data->filter.size; filterX++) {
						size_t imageX =
							(x - data->filter.size / 2 + filterX + data->width) %
							data->width;
						size_t imageY =
							(y - data->filter.size / 2 + filterY + data->height) %
							data->height;

						red +=
							data->input_image->red[imageY * data->width + imageX] *
							data->filter.kernel[filterY][filterX];
						green +=
							data->input_image->green[imageY * data->width + imageX] *
							data->filter.kernel[filterY][filterX];
						blue +=
							data->input_image->blue[imageY * data->width + imageX] *
							data->filter.kernel[filterY][filterX];
					}
				}

				data->output_image->red[y * data->width + x] =
					min(max((int)(data->filter.factor * red + data->filter.bias), 0),
						255);
				data->output_image->green[y * data->width + x] = min(
					max((int)(data->filter.factor * green + data->filter.bias), 0),
					255);
				data->output_image->blue[y * data->width + x] = min(
					max((int)(data->filter.factor * blue + data->filter.bias), 0),
					255);
			}
		}
	}

	pthread_exit(NULL);
}
