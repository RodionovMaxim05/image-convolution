#include "filter_application.h"

void apply_filter(struct image_rgb input_image, struct image_rgb output_image,
				  int width, int height, struct filter filter) {

	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++) {
			double red = 0.0, green = 0.0, blue = 0.0;

			// multiply every value of the filter with corresponding image pixel
			for (int filterY = 0; filterY < filter.size; filterY++)
				for (int filterX = 0; filterX < filter.size; filterX++) {
					int imageX =
						(x - filter.size / 2 + filterX + width) % width;
					int imageY =
						(y - filter.size / 2 + filterY + height) % height;

					red += input_image.red[imageY * width + imageX] *
						   filter.kernel[filterY][filterX];
					green += input_image.green[imageY * width + imageX] *
							 filter.kernel[filterY][filterX];
					blue += input_image.blue[imageY * width + imageX] *
							filter.kernel[filterY][filterX];
				}

			// truncate values smaller than zero and larger than 255
			output_image.red[y * width + x] =
				fmin(fmax((int)(filter.factor * red + filter.bias), 0), 255);
			output_image.green[y * width + x] =
				fmin(fmax((int)(filter.factor * green + filter.bias), 0), 255);
			output_image.blue[y * width + x] =
				fmin(fmax((int)(filter.factor * blue + filter.bias), 0), 255);
		}
}
