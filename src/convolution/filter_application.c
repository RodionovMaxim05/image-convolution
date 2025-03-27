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

void *process_image_part(void *arg) {
	struct thread_data *data = (struct thread_data *)arg;

	printf("start_x: %d, start_y: %d, end_x: %d, end_y: %d\n", data->start_x,
		   data->start_y, data->end_x, data->end_y);

	for (int x = data->start_x; x < data->end_x; x++) {
		for (int y = data->start_y; y < data->end_y; y++) {
			double red = 0.0, green = 0.0, blue = 0.0;

			for (int filterY = 0; filterY < data->filter.size; filterY++) {
				for (int filterX = 0; filterX < data->filter.size; filterX++) {
					int imageX =
						(x - data->filter.size / 2 + filterX + data->width) %
						data->width;
					int imageY =
						(y - data->filter.size / 2 + filterY + data->height) %
						data->height;

					red += data->input_image->red[imageY * data->width + imageX] *
						   data->filter.kernel[filterY][filterX];
					green +=
						data->input_image->green[imageY * data->width + imageX] *
						data->filter.kernel[filterY][filterX];
					blue += data->input_image->blue[imageY * data->width + imageX] *
							data->filter.kernel[filterY][filterX];
				}
			}

			data->output_image->red[y * data->width + x] = min(
				max((int)(data->filter.factor * red + data->filter.bias), 0), 255);
			data->output_image->green[y * data->width + x] = min(
				max((int)(data->filter.factor * green + data->filter.bias), 0), 255);
			data->output_image->blue[y * data->width + x] = min(
				max((int)(data->filter.factor * blue + data->filter.bias), 0), 255);
		}
	}

	pthread_exit(NULL);
}
